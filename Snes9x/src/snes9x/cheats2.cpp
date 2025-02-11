/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <ctype.h>

#include "snes9x.h"
#include "memmap.h"
#include "cheats.h"
#include "bml.h"

#define fopen fopenHelper

static inline char *trim (char *string)
{
    int start;
    int end;

    for (start = 0; string[start] && isspace (string[start]); start++) {}
    for (end = start; string[end] && !isspace (string[end]); end++) {}
    string[end] = '\0';
    return &string[start];
}

static inline uint8 S9xGetByteFree (uint32 Address)
{
    int	block = (Address & 0xffffff) >> MEMMAP_SHIFT;
    uint8 *GetAddress = Memory.Map[block];
    uint8 byte;

    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
    {
        byte = *(GetAddress + (Address & 0xffff));
        return (byte);
    }

    switch ((pint) GetAddress)
    {
    case CMemory::MAP_CPU:
        byte = S9xGetCPU(Address & 0xffff);
        return (byte);

    case CMemory::MAP_PPU:
        if (CPU.InDMAorHDMA && (Address & 0xff00) == 0x2100)
            return (OpenBus);

        byte = S9xGetPPU(Address & 0xffff);
        return (byte);

    case CMemory::MAP_LOROM_SRAM:
    case CMemory::MAP_SA1RAM:
        // Address & 0x7fff   : offset into bank
        // Address & 0xff0000 : bank
        // bank >> 1 | offset : SRAM address, unbound
        // unbound & SRAMMask : SRAM offset
        byte = *(Memory.SRAM + ((((Address & 0xff0000) >> 1) | (Address & 0x7fff)) & Memory.SRAMMask));
        return (byte);

    case CMemory::MAP_LOROM_SRAM_B:
        byte = *(Multi.sramB + ((((Address & 0xff0000) >> 1) | (Address & 0x7fff)) & Multi.sramMaskB));
        return (byte);

    case CMemory::MAP_HIROM_SRAM:
    case CMemory::MAP_RONLY_SRAM:
        byte = *(Memory.SRAM + (((Address & 0x7fff) - 0x6000 + ((Address & 0x1f0000) >> 3)) & Memory.SRAMMask));
        return (byte);

    case CMemory::MAP_BWRAM:
        byte = *(Memory.BWRAM + ((Address & 0x7fff) - 0x6000));
        return (byte);

    case CMemory::MAP_DSP:
        byte = S9xGetDSP(Address & 0xffff);
        return (byte);

    case CMemory::MAP_SPC7110_ROM:
        byte = S9xGetSPC7110Byte(Address);
        return (byte);

    case CMemory::MAP_SPC7110_DRAM:
        byte = S9xGetSPC7110(0x4800);
        return (byte);

    case CMemory::MAP_C4:
        byte = S9xGetC4(Address & 0xffff);
        return (byte);

    case CMemory::MAP_OBC_RAM:
        byte = S9xGetOBC1(Address & 0xffff);
        return (byte);

    case CMemory::MAP_SETA_DSP:
        byte = S9xGetSetaDSP(Address);
        return (byte);

    case CMemory::MAP_SETA_RISC:
        byte = S9xGetST018(Address);
        return (byte);

    case CMemory::MAP_BSX:
        byte = S9xGetBSX(Address);
        return (byte);

    case CMemory::MAP_NONE:
    default:
        byte = OpenBus;
        return (byte);
    }
}

static inline void S9xSetByteFree (uint8 Byte, uint32 Address)
{
    int block = (Address & 0xffffff) >> MEMMAP_SHIFT;
    uint8 *SetAddress = Memory.Map[block];

    if (SetAddress >= (uint8 *) CMemory::MAP_LAST)
    {
        *(SetAddress + (Address & 0xffff)) = Byte;
        return;
    }

    switch ((pint) SetAddress)
    {
    case CMemory::MAP_CPU:
        S9xSetCPU(Byte, Address & 0xffff);
        return;

    case CMemory::MAP_PPU:
        if (CPU.InDMAorHDMA && (Address & 0xff00) == 0x2100)
            return;

        S9xSetPPU(Byte, Address & 0xffff);
        return;

    case CMemory::MAP_LOROM_SRAM:
        if (Memory.SRAMMask)
        {
            *(Memory.SRAM + ((((Address & 0xff0000) >> 1) | (Address & 0x7fff)) & Memory.SRAMMask)) = Byte;
            notifyBackupMemoryWritten();
        }

        return;

    case CMemory::MAP_LOROM_SRAM_B:
        if (Multi.sramMaskB)
        {
            *(Multi.sramB + ((((Address & 0xff0000) >> 1) | (Address & 0x7fff)) & Multi.sramMaskB)) = Byte;
            notifyBackupMemoryWritten();
        }

        return;

    case CMemory::MAP_HIROM_SRAM:
        if (Memory.SRAMMask)
        {
            *(Memory.SRAM + (((Address & 0x7fff) - 0x6000 + ((Address & 0x1f0000) >> 3)) & Memory.SRAMMask)) = Byte;
            notifyBackupMemoryWritten();
        }
        return;

    case CMemory::MAP_BWRAM:
        *(Memory.BWRAM + ((Address & 0x7fff) - 0x6000)) = Byte;
        notifyBackupMemoryWritten();
        return;

    case CMemory::MAP_SA1RAM:
        *(Memory.SRAM + (Address & 0xffff)) = Byte;
        return;

    case CMemory::MAP_DSP:
        S9xSetDSP(Byte, Address & 0xffff);
        return;

    case CMemory::MAP_C4:
        S9xSetC4(Byte, Address & 0xffff);
        return;

    case CMemory::MAP_OBC_RAM:
        S9xSetOBC1(Byte, Address & 0xffff);
        return;

    case CMemory::MAP_SETA_DSP:
        S9xSetSetaDSP(Byte, Address);
        return;

    case CMemory::MAP_SETA_RISC:
        S9xSetST018(Byte, Address);
        return;

    case CMemory::MAP_BSX:
        S9xSetBSX(Byte, Address);
        return;

    case CMemory::MAP_NONE:
    default:
        return;
    }
}

void S9xInitWatchedAddress (void)
{
    for (unsigned int i = 0; i < sizeof(watches) / sizeof(watches[0]); i++)
        watches[i].on = false;
}

void S9xInitCheatData (void)
{
    Cheat.RAM = Memory.RAM;
    Cheat.SRAM = Memory.SRAM;
    Cheat.FillRAM = Memory.FillRAM;
}


void S9xUpdateCheatInMemory (SCheat *c)
{
    uint8 byte;

    if (!c->enabled)
        return;

    byte = S9xGetByteFree (c->address);

    if (byte != c->byte)
    {
        /* The game wrote a different byte to the address, update saved_byte */
        c->saved_byte = byte;

        if (c->conditional)
        {
            if (c->saved_byte != c->cond_byte && c->cond_true)
            {
                /* Condition is now false, let the byte stand */
                c->cond_true = false;
            }
            else if (c->saved_byte == c->cond_byte && !c->cond_true)
            {
                c->cond_true = true;
                S9xSetByteFree (c->byte, c->address);
            }
        }
        else
            S9xSetByteFree (c->byte, c->address);
    }
    else if (c->conditional)
    {
        if (byte == c->cond_byte)
        {
            c->cond_true = true;
            c->saved_byte = byte;
            S9xSetByteFree (c->byte, c->address);
        }
    }
}

void S9xDisableCheat (SCheat *c)
{
    if (!c->enabled)
        return;

    if (!Cheat.enabled)
    {
        c->enabled = false;
        return;
    }

    /* Make sure we restore the up-to-date written byte */
    S9xUpdateCheatInMemory (c);
    c->enabled = false;

    if (c->conditional && !c->cond_true)
        return;

    S9xSetByteFree (c->saved_byte, c->address);
    c->cond_true = false;
}

void S9xDeleteCheatGroup (uint32 g)
{
    unsigned int i;

    if (g >= Cheat.g.size ())
        return;

    for (i = 0; i < Cheat.g[g].c.size (); i++)
    {
        S9xDisableCheat (&Cheat.g[g].c[i]);
    }

    Cheat.g.erase (Cheat.g.begin () + g);
}

void S9xDeleteCheats (void)
{
    unsigned int i;

    for (i = 0; i < Cheat.g.size (); i++)
    {
        S9xDisableCheatGroup (i);
    }

    Cheat.g.clear ();
}

void S9xEnableCheat (SCheat *c)
{
    uint8 byte;

    if (c->enabled)
        return;

    c->enabled = true;

    if (!Cheat.enabled)
        return;

    byte = S9xGetByteFree(c->address);

    if (c->conditional)
    {
        if (byte != c->cond_byte)
            return;

        c->cond_true = true;
    }

    c->saved_byte = byte;
    S9xSetByteFree (c->byte, c->address);
}

void S9xEnableCheatGroup (uint32 num)
{
    unsigned int i;

    for (i = 0; i < Cheat.g[num].c.size (); i++)
    {
        S9xEnableCheat (&Cheat.g[num].c[i]);
    }

    Cheat.g[num].enabled = true;
}

void S9xDisableCheatGroup (uint32 num)
{
    unsigned int i;

    for (i = 0; i < Cheat.g[num].c.size (); i++)
    {
        S9xDisableCheat (&Cheat.g[num].c[i]);
    }

    Cheat.g[num].enabled = false;
}

SCheat S9xTextToCheat (char *text)
{
    SCheat c;
    unsigned int byte = 0;
    unsigned int cond_byte = 0;

    c.enabled     = false;
    c.conditional = false;

    if (!S9xGameGenieToRaw (text, c.address, c.byte))
    {
        byte = c.byte;
    }

    else if (!S9xProActionReplayToRaw (text, c.address, c.byte))
    {
        byte = c.byte;
    }

    else if (sscanf (text, "%x = %x ? %x", &c.address, &cond_byte, &byte) == 3)
    {
        c.conditional = true;
    }

    else if (sscanf (text, "%x = %x", &c.address, &byte) == 2)
    {
    }

    else if (sscanf (text, "%x / %x / %x", &c.address, &cond_byte, &byte) == 3)
    {
        c.conditional = true;
    }

    else if (sscanf (text, "%x / %x", &c.address, &byte) == 2)
    {
    }

    else
    {
        c.address = 0;
        byte = 0;
    }

    c.byte = byte;
    c.cond_byte = cond_byte;

    return c;
}

SCheatGroup S9xCreateCheatGroup (const char *name, const char *cheat)
{
    SCheatGroup g;
    std::string code_string = cheat;
    char *code_ptr = code_string.data();
    int len;

    g.name = name;
    g.enabled = false;

    for (len = strcspn (code_ptr, "+"); len; len = strcspn (code_ptr, "+"))
    {
        char *code = code_ptr;
        code_ptr += len + (code_ptr[len] == '\0' ? 0 : 1);
        code[len] = '\0';
        code = trim (code);

        SCheat c = S9xTextToCheat (code);
        if (c.address)
            g.c.push_back (c);
    }

    return g;
}

int S9xAddCheatGroup (const char *name, const char *cheat)
{
    SCheatGroup g = S9xCreateCheatGroup (name, cheat);
    if (g.c.size () == 0)
        return -1;

    Cheat.g.push_back (g);

    return Cheat.g.size () - 1;
}

int S9xModifyCheatGroup (uint32 num, const char *name, const char *cheat)
{
	if (num >= Cheat.g.size())
		return -1;

    S9xDisableCheatGroup (num);

    Cheat.g[num] = S9xCreateCheatGroup (name, cheat);

    return num;
}

char *S9xCheatToText (SCheat *c)
{
    int size = 10; /* 6 address, 1 =, 2 byte, 1 NUL */
    char *text;

    if (c->conditional)
        size += 3; /* additional 2 byte, 1 ? */

    text = new char[size];

    if (c->conditional)
        snprintf (text, size, "%06x=%02x?%02x", c->address, c->cond_byte, c->byte);
    else
        snprintf (text, size, "%06x=%02x", c->address, c->byte);

    return text;
}

char *S9xCheatGroupToText (SCheatGroup *g)
{
    std::string text = "";
    unsigned int i;

    if (g->c.size () == 0)
        return NULL;

    for (i = 0; i < g->c.size (); i++)
    {
        char *tmp = S9xCheatToText (&g->c[i]);
        if (i != 0)
            text += " + ";
        text += tmp;
        delete[] tmp;
    }

    return strdup (text.c_str ());
}

char *S9xCheatValidate (const char *code_string)
{
    SCheatGroup g = S9xCreateCheatGroup ("temp", code_string);

    if (g.c.size() > 0)
    {
        return S9xCheatGroupToText (&g);
    }

    return NULL;
}

char *S9xCheatGroupToText (uint32 num)
{
    if (num >= Cheat.g.size ())
        return NULL;

    return S9xCheatGroupToText (&Cheat.g[num]);
}

void S9xUpdateCheatsInMemory (void)
{
    unsigned int i;
    unsigned int j;

    if (!Cheat.enabled)
        return;

    for (i = 0; i < Cheat.g.size (); i++)
    {
        for (j = 0; j < Cheat.g[i].c.size (); j++)
        {
            S9xUpdateCheatInMemory (&Cheat.g[i].c[j]);
        }
    }
}

static int S9xCheatIsDuplicate (const char *name, const char *code)
{
    unsigned int i;

    for (i = 0; i < Cheat.g.size(); i++)
    {
        if (!strcmp (name, Cheat.g[i].name.c_str()))
        {
            char *code_string = S9xCheatGroupToText (i);
            char *validated   = S9xCheatValidate (code);

            if (validated && !strcmp (code_string, validated))
            {
                free (code_string);
                free (validated);
                return TRUE;
            }

            free (code_string);
            free (validated);
        }
    }

    return FALSE;
}

static void S9xLoadCheatsFromBMLNode (bml_node *n)
{
    unsigned int i;

    for (i = 0; i < n->child.size (); i++)
    {
        if (!strcasecmp (n->child[i].name.c_str(), "cheat"))
        {
            const char *desc = NULL;
            const char *code = NULL;
            bool8 enabled = false;

            bml_node *c = &n->child[i];
            bml_node *tmp = NULL;

            tmp = c->find_subnode("name");
            if (!tmp)
                desc = (char *) "";
            else
                desc = tmp->data.c_str();

            tmp = c->find_subnode("code");
            if (tmp)
                code = tmp->data.c_str();

            if (c->find_subnode("enable"))
                enabled = true;

            if (code && !S9xCheatIsDuplicate (desc, code))
            {
                int index = S9xAddCheatGroup (desc, code);

                if (enabled)
                    S9xEnableCheatGroup (index);
            }
        }
    }

    return;
}

static bool8 S9xLoadCheatFileClassic (const char *filename)
{
    FILE *fs;
    uint8 data[28];

    fs = fopen(filename, "rb");
    if (!fs)
        return (FALSE);

    while (fread ((void *) data, 1, 28, fs) == 28)
    {
        SCheat c;
        char name[21];
        char cheat[10];
        c.enabled = (data[0] & 4) == 0;
        c.byte = data[1];
        c.address = data[2] | (data[3] << 8) |  (data[4] << 16);
        memcpy (name, &data[8], 20);
        name[20] = 0;

        snprintf (cheat, 10, "%x=%x", c.address, c.byte);
        S9xAddCheatGroup (name, cheat);

        if (c.enabled)
            S9xEnableCheatGroup (Cheat.g.size () - 1);
    }

    fclose(fs);

    return (TRUE);
}

bool8 S9xLoadCheatFile (const char *filename)
{
    bml_node bml;
    if (!bml.parse_file(filename))
    {
        return S9xLoadCheatFileClassic (filename);
    }

   bml_node *n = bml.find_subnode("cheat");
    if (n)
    {
        S9xLoadCheatsFromBMLNode (&bml);
    }

    if (!n)
    {
        return S9xLoadCheatFileClassic (filename);
    }

    return (TRUE);
}

bool8 S9xSaveCheatFile (const char *filename)
{
    unsigned int i;
    FILE *file = NULL;

    if (Cheat.g.size () == 0)
    {
        removeFileHelper(filename);
        return TRUE;
    }

    file = fopen (filename, "w");

    if (!file)
        return FALSE;

    for (i = 0; i < Cheat.g.size (); i++)
    {
        char *txt = S9xCheatGroupToText (i);

        fprintf (file,
                 "cheat\n"
                 "  name: %s\n"
                 "  code: %s\n"
                 "%s\n",
                 Cheat.g[i].name.c_str(),
                 txt,
                 Cheat.g[i].enabled ? "  enable\n" : ""
                 );
        
        free(txt);
    }

    fclose (file);

    return TRUE;
}

void S9xCheatsDisable (void)
{
    unsigned int i;

    if (!Cheat.enabled)
        return;

    for (i = 0; i < Cheat.g.size (); i++)
    {
        if (Cheat.g[i].enabled)
        {
            S9xDisableCheatGroup (i);
            Cheat.g[i].enabled = TRUE;
        }
    }

    Cheat.enabled = FALSE;
}

void S9xCheatsEnable (void)
{
    unsigned int i;

    if (Cheat.enabled)
        return;

    Cheat.enabled = TRUE;

    for (i = 0; i < Cheat.g.size (); i++)
    {
        if (Cheat.g[i].enabled)
        {
            Cheat.g[i].enabled = FALSE;
            S9xEnableCheatGroup (i);
        }
    }
}

int S9xImportCheatsFromDatabase (const char *filename)
{
    char sha256_txt[65];
    char hextable[] = "0123456789abcdef";
    unsigned int i;

    bml_node bml;
    if (!bml.parse_file(filename))
        return -1; // No file

    for (i = 0; i < 32; i++)
    {
        sha256_txt[i * 2]     = hextable[Memory.ROMSHA256[i] >> 4];
        sha256_txt[i * 2 + 1] = hextable[Memory.ROMSHA256[i] & 0xf];
    }
    sha256_txt[64] = '\0';

    for (i = 0; i < bml.child.size (); i++)
    {
        if (!strcasecmp (bml.child[i].name.c_str(), "cartridge"))
        {
            bml_node *n;

            if ((n = bml.child[i].find_subnode ("sha256")))
            {
                if (!strcasecmp (n->data.c_str(), sha256_txt))
                {
                    S9xLoadCheatsFromBMLNode (&bml.child[i]);
                    return 0;
                }
            }
        }
    }

    return -2; /* No codes */
}
