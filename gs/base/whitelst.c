/* Copyright (C) 2001-2012 Artifex Software, Inc.
   All Rights Reserved.

   This software is provided AS-IS with no warranty, either express or
   implied.

   This software is distributed under license and may not be copied,
   modified or distributed except as expressly authorized under the terms
   of the license contained in the file LICENSE in this distribution.

   Refer to licensing information at http://www.artifex.com or contact
   Artifex Software, Inc.,  7 Mt. Lassen Drive - Suite A-134, San Rafael,
   CA  94903, U.S.A., +1(415)492-9861, for further information.
*/


/* $Id: whitelst.c 11608 2010-08-06 11:11:40Z ken $ */
/* The following font names are taken from the Adobe webs site
   (http://www.adobe.com/type/browser/legal/embeddingeula.html)
   and comprise a list of font names for which permission is granted to embed
   either for preview and print or fully editable. These names are used to
   override the embedding flags in TrueType fonts for pdfwrite and ps2write.

   Its not entirely clear whether the font names should include TM and R
   characters or not, and the only example font we have contains neither. For
   the moment the fonts are stored without TM and R characters but this may
   need to be altered.
 */
#include "whitelst.h"

#define WHITE_LIST_SIZE 467

static const char EmbeddingWhiteList[][WHITE_LIST_SIZE] = {
"Aachen",
"Adobe Arabic",
"Adobe Caslon",
"Adobe Devanagari",
"Adobe Fangsong",
"Adobe FanHeiti",
"Adobe Garamond",
"Adobe Gothic",
"Adobe Hebrew",
"Adobe Heiti",
"Adobe Jenson",
"Adobe Kaiti",
"Adobe Ming",
"Adobe Myungjo",
"Adobe Naskh",
"Adobe Song",
"Adobe Text",
"Adobe Thai",
"Adobe Wood Type",
"Albertus",
"Aldus",
"Alexa",
"Americana",
"Amigo",
"Andreas",
"Antique Olive",
"Apollo",
"Arcadia",
"Arcana",
"Ariadne",
"Arno",
"Arnold Boecklin",
"Ashley Script",
"Astrology Pi",
"Audio Pi",
"Auriol",
"Avenir",
"Baker Signet",
"Balzano",
"Banco",
"Banshee",
"Baskerville Cyrillic",
"Bauer Bodoni",
"Bell",
"Bell Centennial",
"Bell Gothic",
"Belwe",
"Bembo",
"Berling",
"Bermuda",
"Bernhard",
"Bernhard Modern",
"Bickham Script",
"Biffo",
"Birch",
"Blackoak",
"Blue Island",
"Bodoni",
"Border Pi",
"Briem Akademi",
"Briem Script",
"Brioso",
"Bruno",
"Brush Script",
"Bulmer",
"Bundesbahn Pi",
"Caflisch Script",
"Calcite",
"Caliban",
"Calvert",
"Candida",
"Cantoria",
"Caravan Borders",
"Carolina",
"Carta",
"Cascade Script",
"Caslon 3",
"Caslon 540",
"Caslon Open Face",
"Castellar",
"Caxton",
"Centaur",
"Century Expanded",
"Century Old Style",
"Chaparral",
"Charlemagne",
"Charme",
"Cheq",
"Clairvaux",
"Clarendon",
"Clearface Gothic",
"Cloister",
"Club Type",
"Cochin",
"Conga Brava",
"Conga Brava Stencil",
"Cooper Black",
"Copal",
"Copperplate Gothic",
"Coriander",
"Corona",
"Coronet",
"Cottonwood",
"Courier",
"Critter",
"Cronos",
"Cutout",
"Dante",
"Decoration Pi",
"Delphin",
"DIN Schriften",
"Diotima",
"Diskus",
"Dom Casual",
"Dorchester Script",
"Doric",
"Duc de Berry",
"Eccentric",
"Egyptienne F",
"Ehrhardt",
"Electra",
"Ellington",
"Else NPL",
"Engravers",
"European Pi",
"Eurostile",
"Ex Ponto",
"Excelsior",
"Fairfield",
"Falstaff",
"Fette Fraktur",
"Flood",
"Florens",
"Flyer",
"Folio",
"Forte",
"Fournier",
"Franklin Gothic",
"Freestyle Script",
"Friz Quadrata",
"Frutiger",
"Fusaka",
"Futura",
"Galahad",
"Game Pi",
"Garamond 3",
"Garamond Premier",
"Garth Graphic",
"Gazette",
"Giddyup",
"Gill Floriated Capitals",
"Gill Sans",
"Glypha",
"Gothic 13",
"Goudy",
"Goudy Text",
"Granjon",
"Graphite",
"Guardi",
"Hadriano",
"Hardwood",
"Heisei Kaku Gothic",
"Heisei Maru Gothic",
"Heisei Mincho",
"Helvetica",
"Helvetica Inserat",
"Helvetica Neue",
"Helvetica Rounded",
"Herculanum",
"Hiroshige",
"Hobo",
"Holiday Pi",
"Horley Old Style",
"HY GungSo",
"HY Kak Headline",
"HY Rounded Gothic",
"Hypatia Sans",
"Immi 505",
"Impact",
"Impressum",
"Industria",
"Inflex",
"Insignia",
"Ironwood",
"Isabella",
"Italia",
"ITC American Typewriter",
"ITC Anna",
"ITC Avant Garde Gothic",
"ITC Bauhaus",
"ITC Beesknees",
"ITC Benguiat",
"ITC Benguiat Gothic",
"ITC Berkeley Oldstyle",
"ITC Bookman",
"ITC Caslon 224",
"ITC Century",
"ITC Century Handtooled",
"ITC Cerigo",
"ITC Cheltenham",
"ITC Cheltenham Handtooled",
"ITC Clearface",
"ITC Cushing",
"ITC Eras",
"ITC Esprit",
"ITC Fenice",
"ITC Flora",
"ITC Franklin Gothic",
"ITC Galliard",
"ITC Garamond",
"ITC Garamond Handtooled",
"ITC Giovanni",
"ITC Goudy Sans",
"ITC Highlander",
"ITC Isadora",
"ITC Kabel",
"ITC Korinna",
"ITC Leawood",
"ITC Legacy Sans",
"ITC Legacy Serif",
"ITC Lubalin Graph",
"ITC Machine",
"ITC Mendoza Roman",
"ITC Mona Lisa",
"ITC Motter Corpus",
"ITC New Baskerville",
"ITC Novarese",
"ITC Officina Sans",
"ITC Officina Serif",
"ITC Ozwald",
"ITC Quorum",
"ITC Serif Gothic",
"ITC Slimbach",
"ITC Souvenir",
"ITC Stone Informal",
"ITC Stone Sans",
"ITC Stone Serif",
"ITC Symbol",
"ITC Tiepolo",
"ITC Tiffany",
"ITC Usherwood",
"ITC Veljovic",
"ITC Weidemann",
"ITC Zapf Chancery",
"ITC Zapf Dingbats",
"Janson Text",
"Jimbo",
"Joanna",
"Juniper",
"Kabel",
"Kaufmann",
"Kazuraki SP2N",
"Kepler",
"Khaki",
"Kigali",
"Kinesis",
"Kino",
"Klang",
"Koch Antiqua",
"Kolo",
"Kompakt",
"Kozuka Gothic",
"Kozuka Mincho",
"Kunstler Script",
"Latin",
"Leander Script ",
"Legault",
"Letter Gothic",
"Life",
"LinoLetter",
"Linoscript",
"Linotext",
"Linotype Centennial",
"Linotype Didot",
"Lithos",
"LogoArl",
"LogoCut",
"LogoLine",
"Lucida",
"Lucida Math",
"Lucida Sans",
"Lucida Sans Typewriter",
"Lucida Typewriter",
"Madrone",
"Manito",
"Marigold",
"Mathematical Pi",
"Matura",
"Maximus",
"Medici Script",
"Melior",
"Memphis",
"Mercurius",
"Meridien",
"Mesquite",
"Mezz",
"MICR",
"Minion",
"Minister",
"Mistral",
"Mojo",
"Monoline Script",
"Monotype Goudy Modern",
"Monotype Grotesque",
"Monotype Italian Old Style",
"Monotype Modern",
"Monotype Old Style",
"Monotype Scotch Roman",
"Monotype Script",
"Montara",
"Moonglow",
"MVB Bossa Nova",
"MVB Celestia Antiqua",
"MVB Emmascript",
"MVB Greymantle",
"MVB Magnesium",
"MVB Magnolia",
"Myriad",
"Myriad Arabic",
"Myriad Hebrew",
"Myriad Hebrew Cursive",
"Mythos",
"National Codes Pi",
"Neue Hammer Unziale",
"Neuland",
"Neuzeit S",
"New Aster",
"New Berolina",
"New Caledonia",
"New Century Schoolbook",
"News Gothic",
"Notre Dame",
"Nueva",
"Nuptial Script",
"Nyx",
"Ocean Sans",
"OCR-A",
"OCR-B",
"Octavian",
"Old Claude",
"Old Style 7",
"Olympian",
"Omnia",
"Ondine",
"Onyx",
"Optima",
"Orator",
"Organica GMM",
"Origami",
"Ouch!",
"Oxford",
"Palace Script",
"Palatino",
"Parisian",
"Park Avenue",
"Peignot",
"Pelican",
"Penumbra Flare",
"Penumbra Half Serif",
"Penumbra Sans",
"Penumbra Serif",
"Pepita",
"Pepperwood",
"Perpetua",
"Photina",
"Plantin",
"PMN Caecilia",
"Poetica",
"Pompeia",
"Pompeijana",
"Ponderosa",
"Poplar",
"Postino",
"Present",
"Prestige Elite",
"Quake",
"Rad",
"Raleigh",
"Raphael",
"Reliq",
"Reporter",
"Revue",
"Rockwell",
"Romic",
"Rosewood",
"Rotation",
"Rotis Sans Serif",
"Rotis Semi Sans",
"Rotis Semi Serif",
"Rotis Serif",
"Ruling Script",
"Runic",
"Russell Oblique",
"Russell Square",
"Rusticana",
"Ruzicka Freehand",
"Ryo Display PlusN",
"Ryo Gothic PlusN",
"Ryo Text PlusN",
"Sabon",
"San Marco",
"Sanvito",
"Sassafras",
"Sava",
"Serifa",
"Serlio",
"Serpentine",
"Shannon",
"Shelley",
"Sho",
"Shuriken Boy",
"Silentium",
"Simoncini Garamond",
"Smaragd",
"SMGothic",
"SMMyungjo",
"Snell Roundhand",
"Sonata",
"Source Sans",
"Spartan",
"Spectrum",
"Spring",
"Spumoni",
"Stempel Garamond",
"Stempel Schneidler",
"Stencil",
"Strayhorn",
"Strumpf",
"Studz",
"Symbol",
"Syntax",
"Tekton",
"Tempo",
"Times",
"Times Europa",
"Times New Roman",
"Times Ten",
"Toolbox",
"Trade Gothic",
"Trajan",
"Trajan Sans",
"Trump Medieval",
"Umbra",
"Univers",
"Universal",
"University",
"Utopia",
"VAG Rounded",
"Vectora",
"Versailles",
"Verve",
"Visigoth",
"Viva",
"Voluta Script",
"Warning Pi",
"Warnock",
"Waters Titling",
"Weiss",
"Wendy",
"Wiesbaden Swing",
"Wilhelm Klingspor Gotisch",
"Wilke",
"Willow",
"Wittenberger Fraktur",
"Zebrawood",
"Zipty Do"
};

static int whitelist_strncmp(const char *s1, const char *s2, int length)
{
    int s1_index, s2_index, result = 0;

    s1_index = s2_index = 0;

    while (s2_index < length && s1[s1_index] != 0x00) {
        while (s1[s1_index] == ' ')
            s1_index++;
        while (s2[s2_index] == ' ' && s2_index < length)
            s2_index++;
        if (s2_index > length) {
            if (s1[s1_index] == 0x00)
                return 0;
            return 1;
        }
        if (s1[s1_index] == 0x00) {
            if (s2_index > length)
                return 0;
            return -1;
        }
        if (s1[s1_index] == s2[s2_index]) {
            s1_index++;
            s2_index++;
            continue;
        }
        if(s1[s1_index] < s2[s2_index])
            return -1;
        if(s1[s1_index] > s2[s2_index])
            return 1;
    }
    return result;
}

int IsInWhiteList (const char *Name, int size)
{
    int low = 0, mid, high = WHITE_LIST_SIZE, test;

    while (low < high) {
        /* bisect current range */
        mid = (low + high) / 2;
        test = whitelist_strncmp(EmbeddingWhiteList[mid], Name, size);
        if (test == 0)
            return 1;
        /* Not a match, select either upper or lower group and try again */
        if(test < 0)
            low = mid + 1;
        else
        high = mid - 1;
    }
    return 0;
}
