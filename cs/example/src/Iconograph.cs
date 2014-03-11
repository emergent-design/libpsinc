using System;
using System.IO;
using System.Linq;
using System.Drawing;
using System.Collections.Generic;

using Xwt;
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
	
	
	class MainWindow : Window
	{
		VBox mainBox		= new VBox();
		ImageView canvas	= new ImageView();
		Acquirer camera		= new Acquirer();

		/*VBox mainBox			= new VBox();
		HBox fileBox			= new HBox();
		HBox serialBox			= new HBox();
		Label statusLabel		= new Label();
		Label subLabel			= new Label();
		Label fileLabel			= new Label("Firmware update file:");
		TextEntry fileEntry		= new TextEntry();
		//ComboBox serialEntry	= new ComboBox();
		Button fileButton		= new Button("...");
		Button flashButton		= new Button("Flash firmware");
		Button serialButton		= new Button("Generate and apply new serial number");
		ProgressBar progress	= new ProgressBar();*/

		
		
		public MainWindow()
		{
			this.Title				= "Iconograph";
			this.Width				= 800;
			this.Height				= 600;
			this.InitialLocation	= WindowLocation.CenterScreen;
			
			/*this.statusLabel.Text			= "No device found";
			this.statusLabel.Font			= this.statusLabel.Font.WithSize(18);
			this.statusLabel.TextAlignment	= Alignment.Center;
			
			this.subLabel.Text			= "";
			this.subLabel.TextAlignment = Alignment.Center;
			
			this.fileEntry.ReadOnly = true;
			this.fileButton.Clicked += FileClicked;
			
			this.flashButton.HeightRequest = 48;
			this.flashButton.Clicked += FlashClicked;*/

			
			/*this.mainBox.PackStart(this.statusLabel, true);
			this.mainBox.PackStart(this.subLabel);
			this.mainBox.PackStart(this.progress);
			this.mainBox.PackStart(new HSeparator() { MarginBottom = 12 });
			//this.mainBox.PackStart(this.heading.Box);
			this.mainBox.PackStart(this.id.Box);
			this.mainBox.PackStart(this.watchdog.Box);
			this.mainBox.PackStart(this.flash.Box);
			this.mainBox.PackStart(this.serialBox);
			this.mainBox.PackStart(new HSeparator() { MarginBottom = 12 });
			this.mainBox.PackStart(this.fileBox);
			this.mainBox.PackStart(this.flashButton);*/

			this.mainBox.PackStart(this.canvas, true);

			this.Content = this.mainBox;
			
			this.CloseRequested += (sender, args) => Application.Exit();

			this.camera.ConnectionChanged	+= this.OnConnection;
			this.camera.Acquired			+= OnAcquired;
			this.camera.Refreshed			+= () => Console.WriteLine("Registers refreshed successfully");
			this.camera.TransferError		+= e => Console.WriteLine(e);
		}


		void OnAcquired(Bitmap image)
		{
			Application.Invoke(() => {
				//this.canvas.Image = image;
			});
		}


		void OnConnection(bool connected)
		{
			Xwt.Drawing.Context c;


		}


		void SetStatus(string label)
		{
		}

		
		protected override void Dispose (bool disposing)
		{
			base.Dispose(disposing);
		}
	}
}