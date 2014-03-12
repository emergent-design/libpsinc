using System;
using System.IO;
using System.Linq;
using System.Collections.Generic;

using Xwt;
using Xwt.Drawing;
using libpsinc;


namespace iconograph
{
	class IconographMain
	{
		[STAThread]
		static void Main()
		{
			Application.Initialize(Environment.OSVersion.Platform == PlatformID.Unix ? ToolkitType.Gtk : ToolkitType.Wpf);
			
			var window = new MainWindow();
			
			window.Show();
			Application.Run();
			window.Dispose();
		}
	}


	class IconographCanvas : Canvas
	{
		Image image;

		DateTime last = DateTime.Now;
		int count = 0;
	

		public Image Image
		{
			set 
			{
				lock (this)
				{
					if (this.image != null) this.image.Dispose();
					this.image = value;

					count++;

					if ((DateTime.Now - last).TotalMilliseconds > 5000)
					{
						Console.WriteLine("{0:0.00} fps", (double)count / 5.0);
						count	= 0;
						last	= DateTime.Now;
					}
				}

				this.QueueDraw();
			}
		}


		protected override void OnDraw(Context ctx, Rectangle dirtyRect)
		{
			lock(this)
			{
				if (this.image != null)
				{
					double scale	= Math.Min((double)this.Size.Width / (double)this.image.Width, (double)this.Size.Height / (double)image.Height);
					double width	= scale * image.Width;
					double height	= scale * image.Height;

					ctx.DrawImage(this.image, 0.5 * (this.Size.Width - width), 0.5 * (this.Size.Height - height), width, height);
				}
			}
		}
	}
	
	
	class MainWindow : Window
	{
		VBox mainBox			= new VBox();
		Label statusLabel		= new Label();
		IconographCanvas canvas	= new IconographCanvas();
		Camera camera			= new Camera();

		
		public MainWindow()
		{
			this.Title						= "Iconograph";
			this.Width						= 800;
			this.Height						= 600;
			this.InitialLocation			= WindowLocation.CenterScreen;
			this.statusLabel.Text			= "Disconnected";
			this.statusLabel.TextAlignment	= Alignment.Center;

			this.mainBox.PackStart(this.canvas, true);
			this.mainBox.PackStart(this.statusLabel, false);

			this.Content 				= this.mainBox;
			this.camera.ImageHandler	= new XwtImageHandler();

			this.CloseRequested 			+= (sender, args) => Application.Exit();
			this.camera.ConnectionChanged	+= this.OnConnection;
			this.camera.Acquired			+= i => this.canvas.Image = i as Image;
			this.camera.TransferError		+= e => Console.WriteLine(e);

			this.camera.Initialise();
		}


		void OnConnection(bool connected)
		{
			if (connected)
			{
				this.camera.Aliases.AutoGain.Value		= 1;
				this.camera.Aliases.AutoExposure.Value	= 1;
				this.camera.Aliases.Exposure.Value		= 10;
			}

			this.SetStatus(connected ? "Connected" : "Disconnected");
		}


		void SetStatus(string label)
		{
			Application.Invoke(() => this.statusLabel.Text = label);
		}

		
		protected override void Dispose (bool disposing)
		{
			this.camera.Dispose();
			base.Dispose(disposing);
		}
	}
}