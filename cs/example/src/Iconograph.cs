using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;

using Gtk;
using libpsinc;
using libpsinc.device;


namespace iconograph
{
	class IconographMain
	{
		[STAThread]
		static void Main()
		{
			Application.Init();
			
			var window = new MainWindow();
			
			window.Show();
			Application.Run();
			window.Dispose();
		}
	}

	
	class MainWindow : Window
	{
		VBox mainBox			= new VBox();
		Label statusLabel		= new Label();
		DrawingArea canvas		= new DrawingArea();
		Camera camera			= new Camera();
		Prox prox				= null;
		LEDArray led			= null;
		
		public MainWindow() : base(WindowType.Toplevel)
		{
			this.Title					= "Iconograph";
			this.WidthRequest			= 800;
			this.HeightRequest			= 600;
			this.WindowPosition			= WindowPosition.Center;
			this.statusLabel.Text		= "Disconnected";
			this.statusLabel.Justify	= Justification.Center;
			this.DeleteEvent 			+= OnDeleteEvent;

			this.mainBox.PackStart(this.canvas, true, true, 4);
			this.mainBox.PackStart(this.statusLabel, false, true, 4);
			this.mainBox.ShowAll();
			this.Add(this.mainBox);

			// For clarity, some optional device initialisation is kept
			// in another function. Take a look if you're interested.
			this.InitialiseDevices();

			// Override the default image handler with a custom Gtk one
			this.camera.ImageHandler = new GtkImageHandler();

			// Set the camera to colour mode (if the connected camera contains a monochrome
			// chip then this will have no effect)
			this.camera.Colour = true;

			// Handle the connection/disconnection of a camera
			this.camera.ConnectionChanged += this.OnConnection;

			// Handle a captured image. The type of object passed in the event depends on
			// the image handler. Since it was overridden above, the image will be a Pixbuf
			// or null if capture failed.
			this.camera.Acquired += i => this.Render(i as Gdk.Pixbuf);

			// Handler a transfer error event by simply writing it to the console
			this.camera.TransferError += e => Console.WriteLine(e);

			// Initialise the camera which will tell it to start looking for
			// devices and begin capturing from them when available
			this.camera.Initialise();
		}



		protected void OnDeleteEvent(object sender, DeleteEventArgs a)
		{
			// The prox device needs to be disposed to ensure
			// that the polling thread exits
			this.prox.Dispose();

			// The camera instance should be disposed when quitting so that
			// it can ensure the internal thread is exited
			this.camera.Dispose();
		
			Application.Quit();
			a.RetVal = true;
		}


		void Render(Gdk.Pixbuf image)
		{
			Application.Invoke(delegate {

				var allocation = this.canvas.Allocation;
				
				using (var context = Gdk.CairoHelper.Create(this.canvas.GdkWindow))
				{
					if (image != null)
					{	
						double scale	= Math.Min((double)allocation.Width / (double)image.Width, (double)allocation.Height / (double)image.Height);
						double width	= scale * image.Width;
						double height	= scale * image.Height;
						
						context.Translate(0.5 * (allocation.Width - width), 0.5 * (allocation.Height - height));
						context.Scale(scale, scale);
						Gdk.CairoHelper.SetSourcePixbuf(context, image, 0, 0);
						context.Paint();

						image.Dispose();
					}
				}
			});
		}
		
		
		void OnConnection(bool connected)
		{
			if (connected)
			{
				// When a new camera is connected, turn on the auto gain and auto exposure.
				this.camera.Aliases.AutoGain.Value		= 1;
				this.camera.Aliases.AutoExposure.Value	= 1;

				// Read the serial number from the camera using a SerialNumber device handler
				Console.WriteLine(new SerialNumber(this.camera).Read());

				// The following section is only really of interest if you want to see
				// how to use the LED array device...

					// Set the LED Array to have a sweeping red light
					this.led.PrimaryMode = LEDArray.Mode.Sweep;
					this.led.PrimaryColour = LEDArray.Colour.Red;
				
					// Flush the LED setting to the camera
					this.led.Flush();
			}

			Application.Invoke((s, e) => this.statusLabel.Text = connected ? "Connected" : "Disconnected");
		}


		protected void InitialiseDevices()
		{
			// An example of how to use some of the more
			// complex devices the camera can connect to.
			// Here we set up a prox card reader and the LED
			// array. When a card is presented we're going to
			// output the ID of the card and then change the
			// light pattern temporarily on the LEDs.

			// Create a new Prox device handler on our camera
			this.prox = new Prox(this.camera);
			
			// Create a new LED Array handler on our camera
			this.led = new LEDArray(this.camera);
			
			// Hook into the CardPresented event on the prox reader.
			// When a card is presented, report the ID of the card
			// on the command line and switch the LED array to green
			this.prox.CardPresented += (ID, data) => {
				Console.WriteLine("Card " + ID + " presented");
				
				// Set the LEDArray to flashing green
				this.led.PrimaryMode = LEDArray.Mode.Flash;
				this.led.PrimaryColour = LEDArray.Colour.Green;
				
				// But set the central LED to constant blue
				this.led.OverrideMode = LEDArray.Override.Central;
				this.led.OverrideColour = LEDArray.Colour.Blue;
				
				// Flush the new settings to the LED device
				this.led.Flush();
				
				// After a couple of seconds, switch the LED
				// array back to red sweeping.
				System.Threading.Timer timer = null;
				timer = new System.Threading.Timer(	c => {
					timer.Dispose();
					this.led.PrimaryMode = LEDArray.Mode.Sweep;
					this.led.PrimaryColour = LEDArray.Colour.Red;
					this.led.OverrideMode = LEDArray.Override.None;
					this.led.Flush();	
				}, null, 3000, System.Threading.Timeout.Infinite);
			};
		}
	}
}