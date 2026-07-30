#pragma once

#include <fstream>
#include <psinc/Transport.h>
#include <psinc/Instrument.h>
#include <emergent/Console.hpp>
#include <emergent/logger/Timestamp.hpp>
#include <entity/entity.hpp>
#include <entity/json.hpp>
#include <flasc/flasc.hpp>
#include <filesystem>
#include <iostream>

// Utilities for flashing multiple cameras based on a set of criteria
namespace flasc
{
	class MultiFlash
	{
		public:

			// anything that is a non-empty string or non-zero integer counts as part of
			// the condition and are effectively ANDed together
			struct Condition
			{
				std::string serial;			// match on serial number if not empty
				std::string id;				// match on ID if not empty
				std::string description;	// match on description if not empty
				std::string firmware;		// match on current firmware if not empty
				uint16_t vendor = 0x00;		// match on vendor ID if not zero
				uint16_t product = 0x00;	// match on product ID if not zero
				bool missing	= false;	// match on empty ID if true

				emap(
					eref(serial), eref(id), eref(description), eref(firmware),
					eref(vendor), eref(product), eref(missing)
				)

				bool Match(const std::string &targetSerial, const std::string &targetId, const std::string &targetFirmware, const Transport::Info &info) const
				{
					return (     this->serial.empty() || this->serial == targetSerial)
						&& (         this->id.empty() || this->id == targetId)
						&& (this->description.empty() || this->description == info.description)
						&& (   this->firmware.empty() || this->firmware == targetFirmware)
						&& (        this->vendor == 0 || this->vendor == info.vendor)
						&& (       this->product == 0 || this->product == info.product)
						&& (       !this->missing     || targetId.empty())
					;
				}

				template <typename T> static void Describe(const std::string &name, const T &target, const T &expected)
				{
					std::cout
						<< emg::Console::Cyan << name << emg::Console::Reset
						<< '(' << target
						<< (target == expected ? "==" : "!=")
						<< expected << ") "
					;
				}

				void Describe(const std::string &targetSerial, const std::string &targetId, const std::string &targetFirmware, const Transport::Info &info) const
				{
					std::cout << "  Comparison: ";
					if (!this->serial.empty()) 		Describe("serial", targetSerial, this->serial);
					if (!this->id.empty())			Describe("id", targetId, this->id);
					if (!this->description.empty())	Describe("description", info.description, this->description);
					if (!this->firmware.empty())	Describe("firmware", targetFirmware, this->firmware);
					if (this->vendor > 0)			Describe("vendor", info.vendor, this->vendor);
					if (this->product > 0)			Describe("product", info.product, this->product);
					if (this->missing)				Describe("missing", targetId.empty(), true);
					std::cout << '\n';
				}
			};


			struct Entry
			{
				std::string firmware;
				std::string version;	// this is the version that should match after a successful flash, perhaps also skip devices that are already up to date?
				std::vector<Condition> conditions;	// an OR set of conditions

				emap(eref(firmware), eref(version), eref(conditions))


				bool Match(const bool debug, const std::string &targetSerial, const std::string &targetId, const std::string &targetFirmware, const Transport::Info &info) const
				{
					for (auto &con : this->conditions)
					{
						if (con.Match(targetSerial, targetId, targetFirmware, info))
						{
							if (debug)
							{
								con.Describe(targetSerial, targetId, targetFirmware, info);
							}
							return true;
						}
					}
					return false;
				}
			};

			// Working state for a camera - so we can keep track of the entry
			// and data that has been read from the device
			struct State
			{
				std::string name;		// which configuration name in the mapping
				Flasc::Basics info;		// basic info read from the device
				Entry entry;			// matching entry
			};


			struct Configuration
			{
				bool dryrun		= false;
				int wait		= 2000;		// Wait time between operations (such as toggling camera mode)
				int attempts	= 2;		// Attempts to flash a device
				bool debug		= false;	// more verbose output if true
				bool log		= true;		// generate a log file

				std::map<std::string, Entry> mapping;

				emap(eref(dryrun), eref(wait), eref(attempts), eref(debug), eref(log), eref(mapping))
			};

			struct Report
			{
				struct Device
				{
					std::string time 	= emg::Timestamp::Now();
					std::string before	= "-";		// firmware before
					std::string after	= "-";		// firmware after
					std::string status;				// success/failed/skipped
					std::string identifier;			// full serial
				};

				std::string time = emg::Timestamp::Now();
				std::vector<Device> devices;
			};


			static bool Go(Flasc &flasc, const Configuration &config, const std::filesystem::path &root)
			{
				if (!CheckConfiguration(flasc, config, root))
				{
					return false;
				}

				Report report;

				const auto cameras = Instrument::List(Instrument::Type::Camera, Instrument::Vendors::All);

				Print(cameras);

				for (const auto &[identifier, info] : cameras)
				{
					Report::Device device;
					device.identifier = identifier;

					if (auto state = Match(flasc, config, identifier, info, device))
					{
						std::cout << emergent::Console::Green << "Handling device " << emg::Console::Reset << identifier << " [configuration = " << state->name << "]\n";

						if (CheckVersion(state.value()))
						{
							std::cout << emg::Console::Green << "  Already up to date, skipping device" << emg::Console::Reset << '\n';
							device.status = "up to date";
						}
						else
						{
							std::cout << "  Flashing device with firmware "<< emg::Console::Cyan << state->entry.firmware << emg::Console::Reset << '\n';

							Flash(flasc, config, identifier, state.value(), root, device);
						}
					}
					else
					{
						device.status = device.status.empty() ? "skipped" : device.status;
						std::cout << emergent::Console::Red << "Skipping device " << emg::Console::Reset << identifier << " [configuration = none]\n";
					}

					std::cout << '\n';
					report.devices.push_back(device);
				}

				Print(std::cout, report);

				if (config.log)
				{
					std::filesystem::path filename = emg::Timestamp::FNow() + "-multiflash.log";
					std::ofstream log(filename);

					if (log.bad())
					{
						std::cout << emg::Console::Red << "Failed to open log file " << filename <<  emg::Console::Reset << '\n';
					}
					else
					{
						Print(log, report);
						std::cout << emg::Console::Green << "Report log saved to " << filename << emg::Console::Reset << '\n';
					}
				}

				return false;
			}

		private:

			static void Flash(Flasc &flasc, const Configuration &config, const std::string &identifier, const State &state, const std::filesystem::path &root, Report::Device &device)
			{
				if (config.dryrun)
				{
					device.status = "skipped (dry run)";
					std::cout << "  Configured for dry run - skipping flash step\n";
					return;
				}

				for (int i=0; i<config.attempts; i++)
				{
					std::cout << emg::Console::Blue << "\n  Attempt " << i+1 << ":\n" << emg::Console::Reset;

					if (flasc.Flash(identifier, (root / state.entry.firmware).string(), config.wait))
					{
						std::cout << "  Checking version number ...\n";
						const auto basics = flasc.BasicInfo(identifier);

						if (basics && !basics->firmware.empty())
						{
							if (state.entry.firmware.empty() || state.entry.version == basics->firmware)
							{
								device.status	= "success";
								device.after 	= basics->firmware;

								std::cout
									<< emg::Console::Green << "  Success" << emg::Console::Reset
									<< "  - device is running expected firmware " << emg::Console::Cyan << basics->firmware << emg::Console::Reset << '\n';
								return;
							}

							std::cout
								<< emg::Console::Yellow << "  Error" << emg::Console::Reset
								<< "  - device is running firmware "
								<< emg::Console::Cyan << basics->firmware << emg::Console::Reset
								<< " but expected " << emg::Console::Cyan << state.entry.version << '\n';
						}
						else
						{
							std::cout << emg::Console::Red << "  Failed to read version information from device\n" << emg::Console::Reset;
						}
					}
				}

				device.status = "failed to update device";
				std::cout << emg::Console::Red << "  Failed to update device\n" << emg::Console::Reset;
			}

			static bool CheckVersion(const State &state)
			{
				std::cout
					<< "  Device is type " << emg::Console::Cyan << state.info.type << emg::Console::Reset
					<< " running firmware " << emg::Console::Cyan << state.info.firmware << emg::Console::Reset << '\n';

				return state.info.firmware == state.entry.version;
			}

			// identifier == full serial (serial:id)
			static std::optional<State> Match(Flasc &flasc, const Configuration &config, const std::string &identifier, const Transport::Info &info, Report::Device &device)
			{
				const auto components		= emg::String::explode(identifier, ":");
				const std::string serial	= components.size() > 0 ? emg::String::trim(components[0], ' ') : "";
				const std::string id		= components.size() > 1 ? emg::String::trim(components[1], ' ') : "";
				const auto basics 			= flasc.BasicInfo(identifier);

				if (basics && !basics->firmware.empty())
				{
					device.before = basics->firmware;

					std::vector<State> states;

					for (auto &[name, entry] : config.mapping)
					{
						if (entry.Match(config.debug, serial, id, basics->firmware, info))
						{
							states.push_back(State {
								name, basics.value(), entry
							});
						}
					}

					if (states.size() > 1)
					{
						device.status = "error - multiple configuration matches";
						std::cout
							<< emg::Console::Red << "  Found multiple configuration matches " << emg::Console::Reset << '['
							<< std::accumulate(
								states.begin() + 1,
								states.end(),
								states.front().name,
								[](std::string c, const State &s) { return std::move(c) + ", " + s.name;	}
							)
							<< "]\n";
						return std::nullopt;
					}

					return states.empty() ? std::nullopt : std::make_optional(states.front());
				}
				else
				{
					device.status = "error - failed to read firmware version";
					std::cout << emg::Console::Red << "  Failed to read current basic information from device" << emg::Console::Reset << '\n';
				}

				return std::nullopt;
			}

			static void Print(const std::map<std::string, Transport::Info> &cameras)
			{
				std::cout
					<< "Discovered devices:\n"
					<< emg::Console::Cyan
					<< std::setw( 6) << "vid"
					<< std::setw( 6) << "pid"
					<< std::setw( 6) << "usb"
					<< std::setw(16) << "description"
					<< "  serial"
					<< emg::Console::Reset << '\n'
				;

				for (const auto &[serial, info] : cameras)
				{
					std::cout
						<< std::setw( 6) << std::hex << info.vendor
						<< std::setw( 6) << std::hex << info.product
						<< std::setw( 6) << info.version
						<< std::setw(16) << emg::String::trim(info.description, '\n')
						<< "  " << serial
						<< '\n'
					;
				}
				std::cout << '\n';
			}

			static void Print(std::ostream &output, const Report &report)
			{
				output
					<< "Firmware update log\n"
					<< "Started: " << report.time << '\n'
					<< "===============================================\n"
				;

				for (auto &device : report.devices)
				{
					output
						<< "           Time: " << device.time << '\n'
						<< "     Identifier: " << device.identifier << '\n'
						<< "Firmware before: " << device.before << '\n'
						<< " Firmware after: " << device.after << '\n'
						<< "         Status: " << device.status << '\n'
						<< "-----------------------------------------------\n"
					;
				}
			}


			static bool CheckConfiguration(Flasc &flasc, const Configuration &config, const std::filesystem::path &root)
			{
				for (auto &[name, entry] : config.mapping)
				{
					if (!std::filesystem::exists(root / entry.firmware))
					{
						return flasc.Abort(String::format("firmware file '%s' for entry '%s' does not exist", entry.firmware, name));
					}
				}

				return true;
			}

	};
}
