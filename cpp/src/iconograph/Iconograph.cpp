#include <gtk/gtk.h>
#include <psinc/Camera.h>
#include <psinc/handlers/ImageHandler.h>
#include <emergent/image/Image.h>
#include <emergent/tinyformat.h>
#include <emergent/Logger.h>
#include <thread>
#include <iostream>
#include <chrono>

using namespace std;
using namespace psinc;
using namespace emergent;
using namespace std::chrono;


struct State
{
	GtkLabel *label					= GTK_LABEL(gtk_label_new("Disconnected"));
	GdkPixbuf *buffer				= nullptr;
	string status					= "Disconnected";
	string framerate				= "0 fps";
	int count						= 0;
	time_point<steady_clock> last	= steady_clock::now();
	Image<byte, rgb> image;
	mutex cs;



	void ConvertImage()
	{
		lock_guard<mutex> guard(this->cs);

		int width	= this->image.Width();
		int height	= this->image.Height();

		if (width > 0 && height > 0)
		{
			if (!buffer || gdk_pixbuf_get_width(buffer) != width || gdk_pixbuf_get_height(buffer) != height)
			{
				if (buffer) g_object_unref(buffer);

				buffer = gdk_pixbuf_new(GDK_COLORSPACE_RGB, false, 8, width, height);
			}

			int stride	= gdk_pixbuf_get_rowstride(buffer);
			int row		= width * 3;
			byte *src	= this->image;
			byte *dst	= gdk_pixbuf_get_pixels(buffer);

			for (int y=0; y<height; y++, src += row, dst += stride)
			{
				memcpy(dst, src, row);
			}
		}
	}


	/*GdkPixbuf *GetPixbuf()
	{
		lock_guard<mutex> guard(this->cs);

		int width	= this->image.Width();
		int height	= this->image.Height();

		if (width > 0 && height > 0)
		{
			auto result = gdk_pixbuf_new(GDK_COLORSPACE_RGB, false, 8, width, height);
			int stride	= gdk_pixbuf_get_rowstride(result);
			int row		= width * 3;
			byte *src	= this->image;
			byte *dst	= gdk_pixbuf_get_pixels(result);

			for (int y=0; y<height; y++, src += row, dst += stride)
			{
				memcpy(dst, src, row);
			}

			return result;
		}

		return nullptr;
	}*/


	bool SetImage(Image<byte, rgb> &image)
	{
		lock_guard<mutex> guard(this->cs);

		this->image = image;

		if (duration_cast<milliseconds>(steady_clock::now() - this->last).count() >= 5000)
		{
			this->framerate	= tfm::format("%.2f fps", (double)this->count / 5.0);
			this->count		= 0;
			this->last 		= steady_clock::now();
		}
		else this->count++;

		return true;
	}


	~State()
	{
		if (buffer) g_object_unref(buffer);
	}
};


gboolean onDraw(GtkWidget *canvas, cairo_t *context, gpointer data)
{
	State *state = (State *)data;

	gtk_label_set_text(state->label, state->status.c_str());

	state->ConvertImage();

	//auto buffer = state->GetPixbuf();

	if (state->buffer)
	//if (buffer)
	{
		int cw			= gtk_widget_get_allocated_width(canvas);
		int ch			= gtk_widget_get_allocated_height(canvas);
		int iw			= gdk_pixbuf_get_width(state->buffer);
		int ih			= gdk_pixbuf_get_height(state->buffer);
		//int iw			= gdk_pixbuf_get_width(buffer);
		//int ih			= gdk_pixbuf_get_height(buffer);
		double scale	= min((double)cw / (double)iw, (double)ch / (double)ih);
		double width	= scale * iw;
		double height	= scale * ih;

		cairo_translate(context, 0.5 * (cw - width), 0.5 * (ch - height));
		cairo_scale(context, scale, scale);
		gdk_cairo_set_source_pixbuf(context, state->buffer, 0, 0);
		//gdk_cairo_set_source_pixbuf(context, buffer, 0, 0);
		cairo_paint(context);

		cairo_text_extents_t extents;
		cairo_identity_matrix(context);
		cairo_select_font_face(context, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_source_rgb(context, 1, 0, 0);
		cairo_text_extents(context, "M", &extents);
		cairo_move_to(context, 4, extents.height);
		cairo_show_text(context, state->framerate.c_str());

		static int counter = 0;

		string count = tfm::format("%d", counter++);
		cairo_move_to(context, 4, extents.height * 2);
		cairo_show_text(context, count.c_str());

		//g_object_unref(buffer);
	}

	return false;
}


GtkWidget *initialiseGui(State &state)
{
	auto window	= gtk_window_new(GTK_WINDOW_TOPLEVEL);
	auto vbox	= gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
	auto canvas	= gtk_drawing_area_new();

	gtk_window_set_title(GTK_WINDOW(window), "Iconograph");
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

	gtk_box_pack_start(GTK_BOX(vbox), canvas, true, true, 4);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(state.label), false, true, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);
	g_signal_connect(canvas, "draw", G_CALLBACK(onDraw), &state);

	gtk_widget_show_all(vbox);
	gtk_widget_show(window);

	return canvas;
}


bool onConnection(string status, State &state, Camera &camera)
{
	bool result		= state.status != status;
	state.status	= status;

	if (status == "Connected")
	{
		camera.aliases[0]["AutoExposure"]->Set(1);
		camera.aliases[0]["AutoGain"]->Set(1);

		cout << camera.devices["Serial"].Read() << endl;
		cout << (string)camera.devices["Name"].Read() << endl;
	}

	return result;
}


int main(int argc, char *argv[])
{
	gtk_init(&argc, &argv);

	logger::instance().add(new sink::console());

	State state;
	Camera camera;
	Image<byte, rgb> image;
	ImageHandler handler(image);

	bool stream		= true;
	bool repaint	= false;
	auto canvas		= initialiseGui(state);

	camera.Initialise();
	camera.GrabImage(Camera::Mode::Master, handler, [&](int status) {
		switch (status)
		{
			case ACQUISITION_SUCCESSFUL: 			repaint = state.SetImage(image);						break;
			case ACQUISITION_CONNECTED:				repaint = onConnection("Connected", state, camera);		break;
			case ACQUISITION_DISCONNECTED:			repaint = onConnection("Disconnected", state, camera);	break;
		}

		if (stream && repaint) gtk_widget_queue_draw(canvas);

		return stream;
	});

	gtk_main();

	// Give the latest asynchronous grab a chance to finish
	// before starting to destroy the local variables
	stream = false;
	while (camera.Grabbing());

	return 0;
}
