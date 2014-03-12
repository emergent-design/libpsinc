using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;

using Gtk;
using libpsinc;


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

		
		public MainWindow() : base(WindowType.Toplevel)
		{
			this.Title						= "Iconograph";
			this.WidthRequest				= 800;
			this.HeightRequest				= 600;
			this.WindowPosition				= WindowPosition.Center;

			this.statusLabel.Text			= "Disconnected";
			this.statusLabel.Justify		= Justification.Center;

			this.mainBox.PackStart(this.canvas, true, true, 4);
			this.mainBox.PackStart(this.statusLabel, false, true, 4);
			this.mainBox.ShowAll();

			this.camera.ImageHandler		= new GtkImageHandler();
			this.camera.Colour				= true;
			this.camera.ConnectionChanged	+= this.OnConnection;
			this.camera.Acquired			+= i => this.Render(i as Gdk.Pixbuf);
			this.camera.TransferError		+= e => Console.WriteLine(e);
			this.DeleteEvent				+= OnDeleteEvent;

			this.Add(this.mainBox);
			this.camera.Initialise();
		}


		protected void OnDeleteEvent(object sender, DeleteEventArgs a)
		{
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
					context.Rectangle(0, 0, allocation.Width, allocation.Height);
					context.SetSourceRGB(1.0, 1.0, 1.0);
					context.Fill();

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

				this.camera.Aliases.AutoGain.Value		= 1;
				this.camera.Aliases.AutoExposure.Value	= 1;
				//this.camera.Aliases.Exposure.Value		= 10;
			}

			Application.Invoke((s, e) => this.statusLabel.Text = connected ? "Connected" : "Disconnected");
		}
	}
}