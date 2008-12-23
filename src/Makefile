#	Makefile
#
#	Copyright 2008 Fred Cooke
#
#	This file is part of the FreeEMS project.
#
#	FreeEMS software is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	FreeEMS software is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with any FreeEMS software.  If not, see <http://www.gnu.org/licenses/>.
#
#	We ask that if you make any changes to this file you send them upstream to us at admin@diyefi.org
#
#	Thank you for choosing FreeEMS to run your engine!

VERSION = 0.0.19

INCDIR = inc
OUTDIR = zoutput
OBJDIR = zobjects
ASMDIR = zassembly
PPCSDIR = zpreprocessedc
# Set the windows/non debian path first and then overwrite
# with the debian one so we can grep the deb line out and
# end up with a working windows make file in an automated
# way. Additionally, please ensure you do not add any sets
# of three .. . in a row or the line will be missing from
# the windows version. Likewise sets of ### longer than 6
# or so are also at risk :-)
PREFIX = m6811-elf-
PREFIX = m68hc11-

CP = cp
RM = rm -f
ZIP = zip
GCC = $(PREFIX)gcc
COPY = $(PREFIX)objcopy
DUMP = $(PREFIX)objdump
NM = $(PREFIX)nm
RE = $(PREFIX)readelf
SIZE = $(PREFIX)size

#Temporary - need to suss out. for now any H change = full recompile.
ASMH = 9S12XDP512asm.s
ISRH = interrupts.h
GLOBALH = freeEMS.h 9S12XDP512.h globalConstants.h globalDefines.h errorDefines.h structs.h tunables.h memory.h
ISRSH = commsISRs.h enginePositionISRs.h ignitionISRs.h injectionISRs.h miscISRs.h realtimeISRs.h
UTILH = utils.h
COMMSH = commsCore.h blockDetailsLookup.h
FUELH = fuelAndIgnitionCalcs.h derivedVarsGenerator.h coreVarsGenerator.h
MAINH = main.h
INITH = init.h
FLASHH = flashWrite.h
LOOKUPH = tableLookup.h
LINKER = memory.x regions.x hc9s12xdp512elfb.x
MFILE = Makefile
WMFILE = Makefile.windows
CINCS = injectorISR.c

# let's keep this to a bare minimum! :-)
HANDASMS = flashBurn.s

# .c files from inc/ should be included here as deps for the parts they are relied on by!! TODO

ALLHEADERS = $(GLOBALH) $(COMMSH) $(ISRSH) $(UTILH) $(FUELH) $(MAINH) $(INITH) $(ISRH) $(FLASHH) $(LOOKUPH)
ALLH = $(patsubst %.h,$(INCDIR)/%.h,$(ALLHEADERS)) $(LINKER) $(MFILE) $(patsubst %.c,$(INCDIR)/%.c,$(CINCS)) $(INCDIR)/$(ASMH)

# Warning : do not include .c files from inc/ as they are included into the other C files and automatically compiled.
#			    0.2k		  2k +				 2k +				2k
FLASHCLASSES = FixedConfig1.c FixedConfig2.c IATTransferTable.c CHTTransferTable.c MAFTransferTable.c TestTransferTable.c
# 8*1k + 8*1k + 8*1k = 24k
FUELCLASSES = FuelTables.c FuelTables2.c
TIMECLASSES = TimingTables.c TimingTables2.c
TUNECLASSES = TunableConfig.c TunableConfig2.c

# Code and global constants can be stored anywhere
SUBCLASSES = fuelAndIgnitionCalcs.c commsCore.c blockDetailsLookup.c coreVarsGenerator.c derivedVarsGenerator.c flashWrite.c tableLookup.c interrupts.c init.c utils.c globalConstants.c
# ISR code that MUST be in the unpaged flash! 
ISRCLASSES = injectionISRs.c ignitionISRs.c enginePositionISRs.c commsISRs.c realtimeISRs.c miscISRs.c

CLASSES = freeEMS.c staticInit.c main.c $(SUBCLASSES) $(ISRCLASSES) $(FLASHCLASSES) $(FUELCLASSES) $(TIMECLASSES) $(TUNECLASSES)

PREPROCESSED = $(patsubst %.c,$(PPCSDIR)/%.pp.c,$(CLASSES))
ASSEMBLIES = $(patsubst %.c,$(ASMDIR)/%.s,$(CLASSES))
OBJECTS = $(patsubst %.c,$(OBJDIR)/%.o,$(CLASSES)) $(patsubst %.s,$(OBJDIR)/%.o,$(HANDASMS))
DUMPS = $(patsubst %.c,$(OBJDIR)/%.dmp,$(CLASSES)) $(patsubst %.s,$(OBJDIR)/%.dmp,$(HANDASMS))

S19FILE = freeems-$(VERSION).s19

REOUT = $(OUTDIR)/freeems-$(VERSION).re
ELFOUT = $(OUTDIR)/freeems-$(VERSION).elf
DUMPOUT = $(OUTDIR)/freeems-$(VERSION).dmp
MAPOUT = $(OUTDIR)/freeems-$(VERSION).map
S19OUT = $(OUTDIR)/$(S19FILE)
S19SAVED = ../firmware/$(S19FILE)
S19ZIP = $(S19SAVED).zip

# M68hc1x GCC Options we may want : -minmax  -mlong-calls

# http://www.gnu.org/software/m68hc11/m68hc11_gcc.html shows code size with different options used
# include debug info, optimise , 16 bit ints, hcs12 platform
GCCOPTS = -g -Wall -Werror -Winline -O -m68hcs12 -mshort -ffunction-sections -fomit-frame-pointer -msoft-reg-count=8 -mauto-incdec -fsigned-char
# -ffunction-sections option splits out the functions such that the garbage collection can get
# them on a per section basis. I'm not sure, but this could be harmful to paged code so may
# have to review this at a later date perhaps splitting paged functions from nonpaged ones.

# pass to linker, link for hc12
LINKOPTS = -Wl,-defsym,vectors_addr=0xF710,-m,m68hc12elfb,-T,hc9s12xdp512elfb.x
LINKOPTSGC = -Wl,--gc-sections,--print-gc-sections,-defsym,vectors_addr=0xF710,-m,m68hc12elfb,-T,hc9s12xdp512elfb.x
#LINKOPTS = -Wl,--print-map,--gc-sections,--print-gc-sections,-defsym,vectors_addr=0xF710,-m,m68hc12elfb,-T,hc9s12xdp512elfb.x
# vectors address in the above line is the VMA address and must be short form 0xF710, not a paged address.
# Add this in : "--print-map," with no spaces/quotes to get a map dump when the linker fails in a confusing way.
# --gc-sections,--print-gc-sections, add those in to find out what isn't being used and clear up flash space

# If using the below option for paged flash, the ADDRESS_TYPE field in the freeEMSloader script should be updated to match.
# --change-section-lma .ppagePP=0xPP8000 \ (where PP = the ppage value for that page of flash) OR if using linear address per formula
#		--only-section=.text2 --change-section-lma .text2=0x380000 \ produced error
# .rodata is now inside .text for convenience during load


COPYOPTS =  --output-target=srec \
		--change-section-lma .fixedconf1+0xFD4000 \
		--change-section-lma .fixedconf2+0xFD4000 \
		     --change-section-lma .text1+0xFD4000 \
		      --change-section-lma .text+0xFEC000 \
		      --change-section-lma .data+0xFEC000 \
		   --change-section-lma .vectors+0xFEC000 \
		    --change-section-lma .rodata+0xFEC000 \
		--only-section=.fixedconf1 \
		--only-section=.fixedconf2 \
		--only-section=.text \
		--only-section=.text1 \
		--only-section=.data \
		--only-section=.vectors \
		--only-section=.rodata \
		--change-section-lma .ppageF8=0xF88000 \
		--change-section-lma .fpageF9=0xF98000 \
		--change-section-lma .dpageF9=0xF9A000 \
		--change-section-lma .ppageFE=0xFE8000 \
		--only-section=.ppageF8 \
		--only-section=.fpageF9 \
		--only-section=.dpageF9 \
		--only-section=.ppageFE \
		--change-section-lma .fpageFA=0xFA8000 \
		--change-section-lma .dpageFA=0xFAA000 \
		--change-section-lma .fpageFB=0xFB8000 \
		--change-section-lma .dpageFB1=0xFBA000 \
		--change-section-lma .dpageFB2=0xFBA400 \
		--change-section-lma .dpageFB3=0xFBA800 \
		--change-section-lma .dpageFB4=0xFBAC00 \
		--change-section-lma .dpageFB5=0xFBB000 \
		--change-section-lma .dpageFB6=0xFBB400 \
		--change-section-lma .dpageFB7=0xFBB800 \
		--change-section-lma .dpageFB8=0xFBBC00 \
		--change-section-lma .fpageFC=0xFC8000 \
		--change-section-lma .dpageFC=0xFCA000 \
		--only-section=.dpageFA \
		--only-section=.fpageFA \
		--only-section=.dpageFB1 \
		--only-section=.dpageFB2 \
		--only-section=.dpageFB3 \
		--only-section=.dpageFB4 \
		--only-section=.dpageFB5 \
		--only-section=.dpageFB6 \
		--only-section=.dpageFB7 \
		--only-section=.dpageFB8 \
		--only-section=.fpageFB \
		--only-section=.dpageFC \
		--only-section=.fpageFC
#		--change-section-lma .ppageFD=0xFD8000 \
#		--change-section-lma .ppageFF=0xFF8000 \
#		--only-section=.ppageFD \	This is left here for clarity only (equivalent to text1 but paged)
#		--only-section=.ppageFF		This is left here for clarity only (equivalent to text but paged)
# Changes to the above options should be reflected in memory.x and memory.h also!
#		--only-section=.ppageE0 \
#		--only-section=.ppageE1 \
#		--only-section=.ppageE2 \
#		--only-section=.ppageE3 \
#		--only-section=.ppageE4 \
#		--only-section=.ppageE5 \
#		--only-section=.ppageE6 \
#		--only-section=.ppageE7 \
#		--only-section=.ppageE8 \
#		--only-section=.ppageE9 \
#		--only-section=.ppageEA \
#		--only-section=.ppageEB \
#		--only-section=.ppageEC \
#		--only-section=.ppageED \
#		--only-section=.ppageEE \
#		--only-section=.ppageEF \
#		--only-section=.ppageF0 \
#		--only-section=.ppageF1 \
#		--only-section=.ppageF2 \
#		--only-section=.ppageF3 \
#		--only-section=.ppageF4 \
#		--only-section=.ppageF5 \
#		--only-section=.ppageF6 \
#		--only-section=.ppageF7 \
#		--change-section-lma .ppageE0=0xE08000 \
#		--change-section-lma .ppageE1=0xE18000 \
#		--change-section-lma .ppageE2=0xE28000 \
#		--change-section-lma .ppageE3=0xE38000 \
#		--change-section-lma .ppageE4=0xE48000 \
#		--change-section-lma .ppageE5=0xE58000 \
#		--change-section-lma .ppageE6=0xE68000 \
#		--change-section-lma .ppageE7=0xE78000 \
#		--change-section-lma .ppageE8=0xE88000 \
#		--change-section-lma .ppageE9=0xE98000 \
#		--change-section-lma .ppageEA=0xEA8000 \
#		--change-section-lma .ppageEB=0xEB8000 \
#		--change-section-lma .ppageEC=0xEC8000 \
#		--change-section-lma .ppageED=0xED8000 \
#		--change-section-lma .ppageEE=0xEE8000 \
#		--change-section-lma .ppageEF=0xEF8000 \
#		--change-section-lma .ppageF0=0xF08000 \
#		--change-section-lma .ppageF1=0xF18000 \
#		--change-section-lma .ppageF2=0xF28000 \
#		--change-section-lma .ppageF3=0xF38000 \
#		--change-section-lma .ppageF4=0xF48000 \
#		--change-section-lma .ppageF5=0xF58000 \
#		--change-section-lma .ppageF6=0xF68000 \
#		--change-section-lma .ppageF7=0xF78000 \


# do the assemblies first as its faster, then objects, then preprocessed files.
# the labels are there to print out the messages, the variables are the dependencies which do the work.
all: preprocess $(PREPROCESSED) compile $(ASSEMBLIES) assemble $(OBJECTS) link $(ELFOUT) debug $(DUMPOUT) $(MAPOUT) $(REOUT) s19 $(S19OUT)
fast: fastcomp $(OBJECTS) link $(ELFOUT) s19 $(S19OUT)
alldebug: dumps $(DUMPS) debug $(DUMPOUT) $(MAPOUT) $(REOUT)
release: save $(S19SAVED) makewinmake $(WMFILE) clean commit svnexp zipall $(ALLZIP) zips19 $(S19ZIP) upload


preprocess:
	################################################################################
	#                        Running the C Pre Processor...                        #
	################################################################################

# Generate preprocessed source files to examine
$(PREPROCESSED): $(PPCSDIR)/%.pp.c: %.c $(ALLH)
	$(GCC) $(GCCOPTS) -E $< > $@


# traditional single stage compile
fastcomp:
	################################################################################
	#                             Compiling FreeEMS...                             #
	################################################################################

#$(ASSEMBLIES): $(ASMDIR)/%.s: $(PPCSDIR)/%.pp.c
	#$(GCC) $(GCCOPTS) -x cpp-output -S -o $@ $<


# generate assembly files to examine
compile:
	################################################################################
	#                         Compiling PPC to Assembly...                         #
	################################################################################

$(ASSEMBLIES): $(ASMDIR)/%.s: $(PPCSDIR)/%.pp.c
	$(GCC) $(GCCOPTS) -x cpp-output -S -o $@ $<


# generate object files to link
assemble:
	################################################################################
	#                          Assembling Object Files...                          #
	################################################################################

$(OBJECTS): $(OBJDIR)/%.o: $(ASMDIR)/%.s
	$(GCC) $(GCCOPTS) -c -o $@ $<


# link the object files into an elf executable
link:
	################################################################################
	#                      Linking the Objects into an ELF...                      #
	################################################################################

$(ELFOUT): $(OBJECTS)
	$(GCC) $(GCCOPTS) $(LINKOPTSGC) -o $(ELFOUT) $(OBJECTS) # ...
	$(GCC) $(GCCOPTS) $(LINKOPTS) -o $(ELFOUT) $(OBJECTS)
# the above is to ensure no warnings, TODO find out how to stop .tramp from being collected!

# dump the full disassembled source with private headers
dumps:
	################################################################################
	#                      Creating Individual Dump Files...                       #
	################################################################################

$(DUMPS): $(OBJDIR)/%.dmp: $(OBJDIR)/%.o
	$(DUMP) -spSd $< > $@


# dump the full disassembled source with private headers
# create a memory map
# output the readelf information 
debug:
	################################################################################
	#                          Generating Debug Files...                           #
	################################################################################
	
$(DUMPOUT): $(ELFOUT)
	$(DUMP) -spSd $(ELFOUT) > $(DUMPOUT)

$(MAPOUT): $(ELFOUT)
	$(NM) $(ELFOUT) | sort > $(MAPOUT)

$(REOUT): $(ELFOUT)
	$(RE) -a -W $(ELFOUT) > $(REOUT)

s19:
	################################################################################
	#                           Building The S19 File...                           #
	################################################################################

# generate a loadable s19
$(S19OUT): $(ELFOUT)
	$(COPY) $(COPYOPTS) $(ELFOUT) $(S19OUT)
	$(SIZE) -t $(OBJECTS)
	echo "Flash image size is" `echo \`cat $(S19OUT) | wc -l\` "* 16" | bc -l` "bytes..."


# not yet implemented!
commit:
	################################################################################
	#                          Commiting final changes...                          #
	################################################################################


# not yet implemented!
svnexp:
	################################################################################
	#                            Exporting from SVN...                             #
	################################################################################


# not yet implemented!
upload:
	################################################################################
	#                            Uploading to SF.net...                            #
	################################################################################


save:
	################################################################################
	#                            Saving The Firmware...                            #
	################################################################################

# save a copy in firmware directory above
$(S19SAVED): $(S19OUT)
	$(CP) $(S19OUT) $(S19SAVED)


makewinmake:
	################################################################################
	#                        Generating Windows Makefile...                        #
	################################################################################

$(WMFILE): $(MFILE)
	../bin/makeWinMake.bash


# zip that copy ready for release
zipall:
	################################################################################
	#                            Zipping The Release...                            #
	################################################################################
	#$(S19ZIP): $(S19SAVED)
	#	$(RM) $(S19ZIP)
	#	$(ZIP) $(S19ZIP) $(S19SAVED)


# zip that copy ready for release
zips19:
	################################################################################
	#                           Zipping The Firmware...                            #
	################################################################################
	# move this process to above the normal dirs
$(S19ZIP): $(S19SAVED)
	$(RM) $(S19ZIP)
	$(ZIP) $(S19ZIP) $(S19SAVED)


# remove the lot
clean:
	################################################################################
	#                 Removing generated pre-processed C files....                 #
	################################################################################
	$(RM) $(PREPROCESSED)
	################################################################################
	#                     Removing generated assembly files....                    #
	################################################################################
	$(RM) $(ASSEMBLIES)
	################################################################################
	#                           Removing object files....                          #
	################################################################################
	$(RM) $(OBJECTS)
	################################################################################
	#                   Removing individual object dump files....                  #
	################################################################################
	$(RM) $(DUMPS)
	################################################################################
	#                        Removing final output files....                       #
	################################################################################
	$(RM) $(ELFOUT)
	$(RM) $(DUMPOUT)
	$(RM) $(MAPOUT)
	$(RM) $(S19OUT)
	$(RM) $(REOUT)