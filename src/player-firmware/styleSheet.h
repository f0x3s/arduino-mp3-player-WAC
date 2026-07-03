// Color Definitions
#define BLACK 0x0000
#define WHITE 0xFFFF
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define GRAY 0x5AEB

// Line Spacing Definitions
#define SINGLE  1.17f
#define ONEPTFIVE  1.5f
#define DOUBLE  2.33f

// Main Menu 
// Colors
#define MM_BG_C BLACK  // Main Menu Background
#define MM_H_TXT_C WHITE  // Main Menu Header Text
#define MM_H_HR_C RED  // Main Menu Header Horizontal Rule

#define MM_TXT_C WHITE  // Main Menu Text
#define MM_TXT_SL_C WHITE  // Main Menu Text Selected

#define MM_HL_C BLUE  // Main Menu Highlight
#define MM_PC_C GRAY  // Main Menu Page Count
#define MM_NP_C GREEN // Main Menu Now Playing
// Spacing
const int MM_MARGIN = 20;
const float MM_LS = SINGLE;  // Main Menu Line Spacing
/// Sizing 
const int MM_H_TXT_SZ = 3;
const int MM_TXT_SZ = 2;


// Sub Menu
// Colors
#define SM_BG_C BLACK // Sub Menu Background
#define SM_H_TXT_C GREEN  // Sub Menu Header Text
#define SM_DES_TXT_C GREEN  // Sub Menu Description Text
#define SM_RET_TXT_C GRAY  // Sub Menu Return Text
// Spacing
const int SM_MARGIN = 20;

// Error
// Colors
#define ER_BG_C RED // Error Screen Background
#define ER_H_TXT_C BLACK  // Error Screen Header Text
#define ER_SH_TXT_C BLACK  // Error Screen Sub Header Text
#define ER_LG_TXT_C BLACK  // Error Screen Log Text
// Spacing
const int ER_MARGIN = 20;  // Error Screen Margins
const float ER_LS = SINGLE;  // Error Screen Line Spacing
//Sizing
const int ER_H_TXT_SZ = 6;  // Error Screen Header Size
const int ER_SH_TXT_SZ = 3;  // Error Screen Sub header Size
const int ER_LG_TXT_SZ = 2;  // Error Screen Sub header Size