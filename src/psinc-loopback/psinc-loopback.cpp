// A simple linux app that captures images from a psinc camera and
// provides raw data to a v4l2 loopback video device.
// Requires
// A build of v4l2loopback from https://github.com/umlaeute/v4l2loopback since the one
// in 18.04 repos is old and broken or simply install the deb package from 20.04.
// then do the following (the exclusive caps makes it possible for browser apps to see the "webcam")
// $ sudo modprobe v4l2loopback exclusive_caps=1

// To persist the kernel module configuration write
//   options v4l2loopback devices=2 exclusive_caps=1
// to /etc/modprobe.d/v4l2loopback.conf
// and then add v4l2loopback to /etc/modules, then
// $ sudo update-initramfs -u

#include <psinc/Camera.h>
#include <psinc/handlers/ImageHandler.hpp>
#include <emergent/String.hpp>
#include <emergent/Timer.hpp>

#include <signal.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <ncurses.h>


using emergent::byte;
using emergent::rgb;
using emergent::String;
using namespace std::chrono_literals;

// todo:
//   connect to specific camera
//   provide a specific /dev/videoN
//   --display frame rate (limit framerate?)--
//   --control exposure/gain/flash/colour?--
//   --auto exposure?--
//   lens correction?


std::string get_format(uint32_t value)
{
	return std::string((char *)&value, 4);
}


void print(v4l2_format &format)
{
	std::cout << "        width = " << format.fmt.pix.width << "\n";
	std::cout << "       height = " << format.fmt.pix.height << "\n";
	std::cout << "       format = " << get_format(format.fmt.pix.pixelformat) << "\n";
	std::cout << "         size = " << format.fmt.pix.sizeimage << "\n";
	std::cout << "        field = " << format.fmt.pix.field << "\n";
	std::cout << "   bytes/line = " << format.fmt.pix.bytesperline << "\n";
	std::cout << "  colourspace = " << format.fmt.pix.colorspace << "\n";
}



struct Output
{
	int fd = 0;
	v4l2_format format;


	bool Open(int width, int height, int depth)
	{
		if (this->fd)
		{
			return true;
		}

		this->fd = open("/dev/video0", O_RDWR);	//O_WRONLY ??

		if (!fd)
		{
			std::cout << "failed to open output\n";
			return false;
		}

		memset(&format, 0, sizeof(format));

		format.type					= V4L2_BUF_TYPE_VIDEO_OUTPUT;
		format.fmt.pix.width		= width;
		format.fmt.pix.height		= height;
		format.fmt.pix.sizeimage	= width * height * depth;
		format.fmt.pix.pixelformat	= depth == 3 ? V4L2_PIX_FMT_RGB24 : V4L2_PIX_FMT_GREY;
		// format.fmt.pix.field		= V4L2_FIELD_NONE;
		// format.fmt.pix.bytesperline	= width * depth;
		// format.fmt.pix.colorspace	= V4L2_COLORSPACE_SRGB;

		if (ioctl(this->fd, VIDIOC_S_FMT, &format) == -1)
		{
			std::cout << "failed to set format: " << strerror(errno) << "\n";

			this->Close();
			return false;
		}

		if (ioctl(this->fd, VIDIOC_G_FMT, &format) == -1)
		{
			std::cout << "failed to read back format: " << strerror(errno) << "\n";

			this->Close();
			return false;
		}

		std::cout << "format configured:\n";
		print(format);

		bool success = format.fmt.pix.width == width
			&& format.fmt.pix.height == height
			&& format.fmt.pix.pixelformat == (depth == 3 ? V4L2_PIX_FMT_RGB24 : V4L2_PIX_FMT_GREY);

		if (!success)
		{
			std::cout << "failed to change the format\n";
			this->Close();
			return false;
		}

		return true;
	}


	void Close()
	{
		if (this->fd)
		{
			close(this->fd);
			this->fd = 0;
		}
	}


	~Output()
	{
		this->Close();
	}

};


struct Colour
{
	double red		= 1.5;
	double green	= 0.96875;
	double blue		= 1.5625;
};

struct Parameters
{
	std::array<Colour, 3> presets = {{
		{ 1.0, 1.0, 1.0 },
		{ 1.5, 1.0625, 1.625 },
		{ 1.0, 0.8125, 2.125 }
	}};

	bool connected	= false;
	int flash		= 0;
	int exposure	= 200;
	int gain		= 3;
	Colour colour	= presets[1];
	int effects		= 0;
	int selected	= 0;
	float fps		= 0.0;
};



double set_colour(psinc::Camera &camera, const std::string &name, const double value)
{
	camera.features[name + "_gain_int"].Set((int)value);
	camera.features[name + "_gain_frac"].Set((int)lrint((value - (int)value) / 0.03125));

	return (double)camera.features[name + "_gain_int"].Get()
		+ 0.03125 * camera.features[name + "_gain_frac"].Get();
}

void set_colour_balance(psinc::Camera &camera, Colour &colour)
{
	colour.red		= set_colour(camera, "red", colour.red);
					  set_colour(camera, "green1", colour.green);
	colour.green	= set_colour(camera, "green2", colour.green);
	colour.blue		= set_colour(camera, "blue", colour.blue);
}


void display(const Parameters &p)
{
	mvprintw(0, 0, p.connected ? String::format("Camera connected at %.1f fps", p.fps).c_str() : "Camera disconnected");
	mvprintw(2, 0, String::format("%c (e)xposure = %d", p.selected == 0 ? '*' : ' ', p.exposure).c_str());
	mvprintw(3, 0, String::format("%c g(a)in     = %d", p.selected == 1 ? '*' : ' ', p.gain).c_str());
	mvprintw(4, 0, String::format("%c (f)lash    = %d", p.selected == 2 ? '*' : ' ', p.flash).c_str());

	mvprintw(6, 0, String::format("%c (r)ed      = %.5f", p.selected == 3 ? '*' : ' ', p.colour.red).c_str());
	mvprintw(7, 0, String::format("%c (g)reen    = %.5f", p.selected == 4 ? '*' : ' ', p.colour.green).c_str());
	mvprintw(8, 0, String::format("%c (b)lue     = %.5f", p.selected == 5 ? '*' : ' ', p.colour.blue).c_str());
	mvprintw(10, 0, String::format("%c f(x)      = %d", p.selected == 6 ? '*' : ' ', p.effects).c_str());
}


void update(psinc::Camera &camera, Parameters &p)
{
	camera.SetFlash(p.flash);
	camera.aliases[0].exposure->Set(p.exposure);
	camera.aliases[0].gain->Set(p.gain);

	set_colour_balance(camera, p.colour);

	p.exposure	= camera.aliases[0].exposure->Get();
	p.gain		= camera.aliases[0].gain->Get();
}



bool stream = true;


void adjust(Parameters &p, int direction)
{
	switch (p.selected)
	{
		case 0:	p.exposure		+= direction * 10;		break;
		case 1: p.gain			+= direction;			break;
		case 2: p.flash			+= direction;			break;
		case 3: p.colour.red	+= 0.03215 * direction;	break;
		case 4: p.colour.green	+= 0.03215 * direction;	break;
		case 5: p.colour.blue	+= 0.03215 * direction;	break;
		case 6: p.effects		+= direction;			break;
	}

	p.flash		= std::max(0, std::min(p.flash, 255));
	p.effects	= std::max(0, std::min(p.effects, 3));
}


bool keys(Parameters &p)
{
	int ch = wgetch(stdscr);

	if (ch == ERR)
	{
		return false;
	}

	switch (ch)
	{
		case 'q': stream = false;	break;
		case 'e': p.selected = 0;	break;
		case 'a': p.selected = 1;	break;
		case 'f': p.selected = 2;	break;
		case 'r': p.selected = 3;	break;
		case 'g': p.selected = 4;	break;
		case 'b': p.selected = 5;	break;
		case 'x': p.selected = 6;	break;

		case '0': p.colour 	= p.presets[0];	break;
		case '1': p.colour 	= p.presets[1];	break;
		case '2': p.colour 	= p.presets[2];	break;

		case '-': adjust(p, -1);	break;
		case '=': adjust(p,  1);	break;
		case '+': adjust(p,  1);	break;

		case '\033':
			getch();	// skip escape code '['
			switch(getch())
			{
				case 'A': p.selected = std::max(0, p.selected - 1);	break;
				case 'B': p.selected = std::min(6, p.selected + 1); break;
				case 'C': adjust(p, 1);		break;
				case 'D': adjust(p, -1);	break;
			}
			break;
	}

	return true;
}


// Not to be left in
// #include <imp/fiducials/Detector.h>
// #include <imp/filter/EdgeDetect.h>
// #include <imp/feature/EdgeRefine.h>
// #include <imp/feature/ObjectExtract.h>
// #include <imp/image/Draw.hpp>
// #include <imp/image/Text.h>


// void process_fiducials(Output &output, emg::Image<byte, rgb> &src)
// {
// 	static const auto format = imp::Text::Format().Size(18).Colour(emg::RGB::Cyan).Background(emg::RGB::Black).Align(imp::Alignment::Auto).Offset(4);

// 	static imp::Text text;
// 	static imp::fiducials::Detector detector;
// 	static emg::Image<byte> buffer;
// 	static emg::Image<byte, rgb> dst;

// 	buffer = dst = src;

// 	for (auto &f : detector.Find({}, buffer))
// 	{
// 		if (f.code < 0)
// 		{
// 			imp::Draw::Ellipse(dst, emg::RGB::Magenta, f.base);
// 			imp::Draw::Cross(dst, emg::RGB::Magenta, f.position, 2);
// 		}
// 		else
// 		{
// 			const auto &m	= f.base.matrix;
// 			double dx		= 0.75 * cos(f.orientation);
// 			double dy		= 0.75 * sin(f.orientation);
// 			double x		= f.position.x + m[0] * dx + m[1] * dy;
// 			double y		= f.position.y + m[2] * dx + m[3] * dy;

// 			imp::Draw::SmoothLine(dst, emg::RGB::Cyan, f.position.x, f.position.y, x, y);

// 			text.Render(dst, String::format("%d", f.code), format, lrint(f.position.x), lrint(f.position.y));
// 		}
// 	}

// 	write(output.fd, dst.Data(), dst.Size() * 3);
// }


// void process_edge(Output &output, emg::Image<byte, rgb> &src)
// {
// 	static imp::EdgeDetect edge;
// 	static imp::EdgeRefine refine;
// 	static imp::ObjectExtract oe;
// 	static emg::Image<byte> grey, buffer;
// 	static emg::Image<byte, rgb> dst;

// 	dst.Resize(src.Width(), src.Height());
// 	dst		= 0;
// 	grey	= src;

// 	edge.FindEdges(&grey, &buffer, -1);
// 	auto objects = oe.Search(&buffer);
// 	refine.Process(&grey, objects);

// 	dst = src;
// 	imp::Draw::Objects(dst, emg::RGB::White, objects);

// 	write(output.fd, dst.Data(), dst.Size() * 3);
// }


void process_invert(Output &output, emg::Image<byte, rgb> &src)
{
	src.Invert();
	write(output.fd, src.Data(), src.Size() * 3);
}


void process(Parameters &p, Output &output, emg::Image<byte, rgb> &src)
{
	static emg::Timer timer;
	static int count = 0;

	switch (p.effects)
	{
		case 1: process_invert(output, src);					break;
		// case 2: process_edge(output, src);						break;
		// case 3: process_fiducials(output, src);					break;
		default: write(output.fd, src.Data(), src.Size() * 3);	break;
	}

	if (timer.Elapsed() > 2000)
	{
		p.fps = 1000.0 * count / timer.Elapsed();
		count = 0;
		timer.Reset();
	}
	else
	{
		count++;
	}
}


int main(int argc, char *argv[])
{
	signal(SIGINT,	[](int) { stream = false; });
	signal(SIGQUIT,	[](int) { stream = false; });
	signal(SIGTERM,	[](int) { stream = false; });

	Output output;
	Parameters parameters;

	psinc::Camera camera;
	emg::Image<byte, rgb> buffer;
	psinc::ImageHandler<byte> handler(buffer);

	camera.Initialise("", [&](bool connected) {
		if (connected)
		{
			parameters.connected = true;
			update(camera, parameters);
		}
		else
		{
			output.Close();
			parameters.connected = false;
		}
	});


	camera.GrabImage(psinc::Camera::Mode::Normal, handler, [&](bool success) {

		if (success)
		{
			if (!output.fd)
			{
				if (!output.Open(buffer.Width(), buffer.Height(), buffer.Depth()))
				{
					return stream = false;
				}
			}

			process(parameters, output, buffer);
			// write(output.fd, buffer.Data(), buffer.Size() * 3);
		}

		return stream;
	});


	initscr();
	noecho();
	nodelay(stdscr, true);

	while (stream)
	{
		if (keys(parameters))
		{
			update(camera, parameters);
		}

		wclear(stdscr);// werase(stdscr);
		display(parameters);
		wrefresh(stdscr);

		std::this_thread::sleep_for(10ms);
	}

	endwin();

	while (camera.Grabbing())
	{
		std::this_thread::sleep_for(10ms);
	}
}

