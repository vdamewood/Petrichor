/* command.c: Command lookup
 *
 * Copyright 2015, 2016 Vincent Damewood
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stddef.h>

#include "acpi.h"
#include "command.h"
#include "cpuid.h"
//#include "fat12.h"
//#include "floppy.h"
#include "mboot.h"
#include "memory.h"
#include "screen.h"
#include "timer.h"
#include "uio.h"
#include "util.h"

static int ClearScreen(int, char *[]);
static int GreetUser(int, char *[]);
static int ShowHelp(int, char *[]);
static int Stub(int, char *[]);
static int Vendor(int, char *[]);
static int MemoryMap(int, char *[]);
static int AcpiHeaders(int, char *[]);
static int Shutdown(int, char *[]);
static int TestArgs(int, char *[]);
static int Color(int, char *[]);
//static int TestFloppy(int, char*[]);
//static int Dir(int, char*[]);
static int BootInfo(int, char *[]);

struct entry
{
	char *command;
	char *help;
	int (*routine)(int,char*[]);
};
typedef struct entry entry;

entry CommandTable[] =
{
	{"hi",       "Display a greeting",        GreetUser},
	{"clear",    "Clear the screen",          ClearScreen},
	{"vendor",   "Display vendor from CPUID", Vendor},
	{"memory",   "Show a map of memory",      MemoryMap},
	{"acpi",     "Show acpi headers",         AcpiHeaders},
	{"help",     "Show this help",            ShowHelp},
	{"shutdown", "Turn the system off",       Shutdown},
	{"test",     "test arguments",            TestArgs},
	{"color",    "change color",              Color},
//	{"floppy",   "test floppy drive",         TestFloppy},
//	{"dir",      "Show a directory",          Dir},
//	{"load",     "Load a file",               Load},
	{"bootinfo", "Show Bootloader info",      BootInfo},
	{0,          0,                           Stub}
};


// FIXME: Implement memory allocation
#define argMax 8
#define argSize 24
static char argumentBuffer[argMax][argSize];
char *argumentPointers[argMax];


int (*cmdGet(const char *in))(int,char*[])
{
	for (entry *candidate = CommandTable; candidate->command != NULL; candidate++)
			if (blStrCmp(in, candidate->command) == 0)
				return candidate->routine;
	return NULL;
}

static int GreetUser(int argc, char *argv[])
{
	uioPrint("Hello.\n");
	return 0;
}

static int TestArgs(int argc, char *argv[])
{
	for (int i=0; i<argc; i++)
	{
		uioPrint(argv[i]);
		uioPrintChar('\n');
	}

	return 0;
}

static int ClearScreen(int argc, char *argv[])
{
	scrClear();
	return 0;
}

static int Vendor(int argc, char *argv[])
{
	cpuidShowVendor();
	return 0;
}

static int MemoryMap(int argc, char *argv[])
{
	memShowMap();
	return 0;
}

static int AcpiHeaders(int argc, char *argv[])
{
	AcpiShowHeaders();
	return 0;
}

static int Shutdown(int argc, char *argv[])
{
	AcpiShutdown();
	return 0;
}

struct colorTableEntry
{
	char *name;
	unsigned char value;
};


struct colorTableEntry colors[] =
{
	{"black",   scrBlack},
	{"blue",    scrBlue},
	{"green",   scrGreen},
	{"cyan",    scrCyan},
	{"red",     scrRed},
	{"purple",  scrPurple},
	{"yellow",  scrYellow},
	{"white",   scrWhite},
	{"black+",  scrBright | scrBlack},
	{"blue+",   scrBright | scrBlue},
	{"green+",  scrBright | scrGreen},
	{"cyan+",   scrBright | scrCyan},
	{"red+",    scrBright | scrRed},
	{"purple+", scrBright | scrPurple},
	{"yellow+", scrBright | scrYellow},
	{"white+",  scrBright | scrWhite},
	{0,         0xFF}
};

static int Color(int argc, char *argv[])
{
	if (argc == 3)
	{
		unsigned char setColor = 0xFF;

		for (struct colorTableEntry *candidate = colors; candidate->name != 0; candidate++)
			if (blStrCmp(candidate->name, argv[2]) == 0)
				setColor = candidate->value;

		if (setColor != 0xFF)
			if (blStrCmp(argv[1], "text") == 0)
				scrSetForgroundColor(setColor);

			else if (blStrCmp(argv[1], "highlight") == 0)
				scrSetBackgroundColor(setColor);
			else
				uioPrint("Bad color target\n");
		else
			uioPrint("Bad color\n");
	}
	else
	{
		uioPrint("bad arguments\n");
	}
	return 0;
}


char buffer[80] = "";
/*static int TestFloppy(int argc, char *argv[])
{
	uioPrint("Testing floppy drive initialization.\n");
	drvStorageDevice floppy = fdGetDriver();
	uioPrint("Testing floppy drive read.");
	floppy.ReadSectors(floppy.Driver.State, 0, 1, (void*)0x500);

	uioPrint("Checking values:\n");

	char *buffers[3] = {(char*)0x7C00, fdGetBuffer(), (char*)0x500};

	for (int i = 0; i < 3; i++)
	{
		char *byte = buffers[i];
		char *limit = byte+0x18;
		do
		{
			uioPrintHexByte(*byte++);
			uioPrintChar(' ');
		}
		while(byte < limit);
		uioPrintChar('\n');
	}
	return 0;
}*/

static int ShowHelp(int argc, char *argv[])
{
	static int maxLen = 0;

	if (!maxLen)
		for (entry *candidate = CommandTable; candidate->command != 0; candidate++)
		{
			int current = blStrLen(candidate->command);
			if (current > maxLen)
				maxLen = current;
		}

	for (entry *candidate = CommandTable; candidate->command != 0; candidate++)
	{
		uioPrint(candidate->command);
		for (int i = maxLen + 2 - blStrLen(candidate->command); i != 0; i--)
			uioPrintChar(' ');

		uioPrint(" -- ");
		uioPrint(candidate->help);
		uioPrintChar('\n');
	}
	return 0;
}

/*static int Dir(int argc, char *argv[])
{
	const char *directory = "/";
	drvStorageDevice floppy = fdGetDriver();

	if (argc == 2)
		directory = argv[1];

	fat12ShowDirectory(&floppy, directory);
	return 0;
}*/


static int BootInfo(int argc, char *argv[])
{
	uioPrint("EAX: ");
	uioPrintHexDWord(SavedEax);
	uioPrint("\n");
	uioPrint("EBX: ");
	uioPrintHexDWord(SavedEbx);
	uioPrint("\n");

	uioPrint("Flags: ");
	uioPrintHexDWord(MbInfo.flags);
	uioPrint("\n");

	if (MbInfo.flags & MB_FLAG_MEMORY)
	{
		uioPrint("Memory: Lower: ");
		uioPrintHexDWord(MbInfo.mem_lower);
		uioPrint("k; Upper: ");
		uioPrintHexDWord(MbInfo.mem_upper);
		uioPrint("k\n");
	}

	if (MbInfo.flags & MB_FLAG_BOOTDEV)
	{
		uioPrint("Boot: ");
		uioPrintHexByte((unsigned char)(MbInfo.boot_device));
		uioPrint("(");
		uioPrintHexByte((unsigned char)(MbInfo.boot_parts[2]));
		uioPrint(":");
		uioPrintHexByte((unsigned char)(MbInfo.boot_parts[1]));
		uioPrint(":");
		uioPrintHexByte((unsigned char)(MbInfo.boot_parts[0]));
		uioPrint(")\n");
	}

	if (MbInfo.flags & MB_FLAG_COMMAND)
	{
		uioPrint("Command: ");
		uioPrint(MbInfo.cmdline);
		uioPrint("\n");
	}

	if (MbInfo.flags & MB_FLAG_BOOTMODS)
	{
		uioPrint("Booted Modules: ");
		uioPrintHexDWord(MbInfo.mods_count);
		uioPrint("\n");

		for (unsigned int i = 0; i < MbInfo.mods_count; i++)
		{
			uioPrintHexDWord(i);
			uioPrint(" ");
			uioPrint(MbInfo.mods[i].string);
			uioPrint("\n");
		}

	}

	if (MbInfo.flags & MB_FLAG_AOUTIMG)
	{
		uioPrint("Bootloader passed a.out data, but this kernel doesn't support a.out.\n");
	}

	if (MbInfo.flags & MB_FLAG_ELFIMG)
	{
		uioPrint("ELF: ");
		uioPrintHexDWord(MbInfo.Elf.num);
		uioPrintHexDWord(MbInfo.Elf.size);
		uioPrintHexDWord(MbInfo.Elf.addr);
		uioPrintHexDWord(MbInfo.Elf.shndx);
		uioPrint("\n");
	}

	if (MbInfo.flags & MB_FLAG_MMAP)
	{
		uioPrint("MMAP: Length: ");
		uioPrintHexDWord(MbInfo.mmap_length);
		uioPrint("; Address: ");
		uioPrintHexDWord(MbInfo.mmap_addr);
		uioPrint("\n");
	}

	if (MbInfo.flags & MB_FLAG_DRIVES)
	{
		uioPrint("drives_length: ");
		uioPrintHexDWord(MbInfo.drives_length);
		uioPrint("\n");

		uioPrint("drives_addr: ");
		uioPrintHexDWord(MbInfo.drives_addr);
		uioPrint("\n");
	}

	if (MbInfo.flags & MB_FLAG_BIOSCONF)
	{
		uioPrint("config_table: ");
		uioPrintHexDWord(MbInfo.config_table);
		uioPrint("\n");
	}

	if (MbInfo.flags & MB_FLAG_BOOTLDR)
	{
		uioPrint("Bootloader: ");
		uioPrint(MbInfo.boot_loader_name);
		uioPrint("\n");
	}

	if (MbInfo.flags & MB_FLAG_APMTBL)
	{
		uioPrint("apm_table: ");
		uioPrintHexDWord(MbInfo.apm_table);
		uioPrint("\n");
	}

	if (MbInfo.flags & MB_FLAG_GRAPHICS)
	{
		uioPrint("vbe_control_info: ");
		uioPrintHexDWord(MbInfo.vbe_control_info);
		uioPrint("\n");

		uioPrint("vbe_mode_info: ");
		uioPrintHexDWord(MbInfo.vbe_mode_info);
		uioPrint("\n");

		uioPrint("vbe_mode: ");
		uioPrintHexDWord(MbInfo.vbe_mode);
		uioPrint("\n");

		uioPrint("vbe_interface_seg: ");
		uioPrintHexDWord(MbInfo.vbe_interface_seg);
		uioPrint("\n");

		uioPrint("vbe_interface_off: ");
		uioPrintHexDWord(MbInfo.vbe_interface_off);
		uioPrint("\n");

		uioPrint("vbe_interface_len: ");
		uioPrintHexDWord(MbInfo.vbe_interface_len);
		uioPrint("\n");
	}
	return 0;
}

static int Stub(int argc, char *argv[])
{
	return 0;
}
