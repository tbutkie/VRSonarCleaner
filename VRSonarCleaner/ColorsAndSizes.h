//SMT FLOW VARIABLES:

//MAIN VIS STUFF:

#define MAIN_BACKGROUND_COLOR 0.33,0.39,0.49


//PANEL STUFF
#define PANEL_EDGE_COLOR 0.18, 0.25, 0.35
#define PANEL_EDGE_ALPHA 0.90
#define PANEL_EDGE_WIDTH 1

#define PANEL_FILL_COLOR  0.47,0.54,0.65
#define PANEL_FILL_ALPHA  0.75

#define PANEL_LABEL_COLOR 0.0,0.0,0.0
#define PANEL_LABEL_LINE_WIDTH 1
#define PANEL_LABEL_FILL_COLOR 0.67,0.74,0.85
#define PANEL_LABEL_FILL_ALPHA  0.90

#define INACTIVE_WINDOW_ALPHA_PENALTY .30

#define PANEL_EDGE_PADDING 3

#define TITLE_BAR_HEIGHT 20

#define DYE_POLE_COLOR 0.65,0.65,0.65


#define BUTTON_SIZE 40
#define BUTTON_LINE_COLOR 0.15,0.15,0.30
#define BUTTON_OUTLINE_COLOR 0.15,0.15,0.15
#define BUTTON_COLOR 0.55, 0.61, 0.70//0.67,0.74,0.85
#define BUTTON_COLOR_INACTIVE 0.47,0.54,0.65
#define BUTTON_LABEL_LINE_WIDTH 1


#define PANEL_TEXT_COLOR 1,1,1
#define PANEL_TEXT_WIDTH 1
#define PANEL_TEXT_SCALE 0.075

#define SLIDER_BAR_LINE_COLOR 0.05,0.2,0.4
#define SLIDER_BAR_COLOR 0.53,0.60,0.78



//PARTICLE SYSTEM STUFF

#define MIN_DYE_RADIUS 0.1
#define DEFAULT_DYE_RADIUS 1
#define MAX_DYE_RADIUS 5

#define MIN_DYE_RATE 1
#define DEFAULT_DYE_RATE 500
#define MAX_DYE_RATE 5000

#define MIN_DYE_LIFETIME 1000ms
#define DEFAULT_DYE_LIFETIME 10000ms
#define MAX_DYE_LIFETIME 60000ms

#define MIN_DYE_LENGTH 250ms
#define DEFAULT_DYE_LENGTH 1000ms
#define MAX_DYE_LENGTH 5000ms

#define MIN_GRAVITY -100.0
#define DEFAULT_GRAVITY 0.0
#define MAX_GRAVITY 100.0

#define MIN_NUM_SEEDS_TO_MAINTAIN 0
#define DEFAULT_NUM_SEEDS_TO_MAINTAIN 20000
#define MAX_NUM_SEEDS_TO_MAINTAIN 100000



//color 0 - light blue/cyan (classic streaklet)
#define COLOR_0_R 0.25
#define COLOR_0_G 0.95
#define COLOR_0_B 1.0

//color 1 - yellow
#define COLOR_1_R 1.0
#define COLOR_1_G 1.0
#define COLOR_1_B 0.0

//color 2 - green
#define COLOR_2_R 0.0
#define COLOR_2_G 1.0
#define COLOR_2_B 0.0

//color 3 - magenta
#define COLOR_3_R 1.0
#define COLOR_3_G 0.0
#define COLOR_3_B 1.0

//color 4 - orange
#define COLOR_4_R 1.0
#define COLOR_4_G 0.5
#define COLOR_4_B 0.0

//color 5 - violet
#define COLOR_5_R 0.62
#define COLOR_5_G 0.32
#define COLOR_5_B 1.0

//color 6 - fuscia
#define COLOR_6_R 1.0
#define COLOR_6_G 0.0
#define COLOR_6_B 0.5

//color 7 - golden
#define COLOR_7_R 0.72
#define COLOR_7_G 0.65
#define COLOR_7_B 0.0

//color 8 - pink
#define COLOR_8_R 1.0
#define COLOR_8_G 0.5
#define COLOR_8_B 0.5


//////////MAP STUFF//////////////
#define COASTAL_SHAPEFILE_COLOR  0.20,0.20,0.25
#define CITIES_COLOR 0.60, 0.60, 0.75
#define NUKEPLANTS_COLOR 0.75, 0.75, 0.0



////////////////


//#define TITLE_BAR_LEFT_OFFSET 5
//#define TITLE_BAR_RIGHT_OFFSET 5





/*



//touchtable:
//#define INITIAL_WINDOW_SIZE_X 2160
//#define INITIAL_WINDOW_SIZE_Y 1920

#define INERTIAL_DAMPENING_FACTOR 2

//30" LCD
#define INITIAL_WINDOW_SIZE_X 2550
#define INITIAL_WINDOW_SIZE_Y 1535

#define BUTTON_REPRESS_TIME 250 //milliseconds

#define BACKGROUND_COLOR 0.33,0.39,0.49,1.0
#define BACKGROUND_R 0.33
#define BACKGROUND_G 0.39
#define BACKGROUND_B 0.49

#define ACRES_PER_PIXEL 3.829814 //0.22 for full resolution (8545x6200)  //16.05424 for small res (1000x726)  //3.829814 for gold res 2048x1486

//#define MAP_ORIGIN_X 311985
//#define MAP_ORIGIN_Y 80416
//#define MAP_SIZE_X	 253770
//#define MAP_SIZE_Y	 185430

#define ANIMATION_SPEED_SECONDS_PER_FRAME 0.1
#define ANIMATION_SPEED_FADE_INCREMENT_SLOW .025
#define ANIMATION_SPEED_FADE_INCREMENT_MEDIUM .05
#define ANIMATION_SPEED_FADE_INCREMENT_FAST .1
#define ANIMATION_BUTTON_SPIN_SPEED_SLOW 0.75
#define ANIMATION_BUTTON_SPIN_SPEED_MEDIUM 1.5
#define ANIMATION_BUTTON_SPIN_SPEED_FAST 3

#define MAP_ORIGIN_X 311372.178204
#define MAP_ORIGIN_Y 80415.9427485
#define MAP_SIZE_X	 256350
#define MAP_SIZE_Y	 186000

//initial draw location for base map
#define DEFAULT_MAP_X 288
#define DEFAULT_MAP_Y 50
#define DEFAULT_MAP_WIDTH 1974.7978
#define DEFAULT_MAP_HEIGHT 1435

#define YEAR_LABEL_COLOR 0.208, 0.467, 0.78
#define YEAR_LABEL_LINE_WIDTH 5


#define MAX_NUMBER_OF_TEXTURES 64
#define NUM_YEARS 9
#define YEAR_76_TEXTURE_INDEX 0
#define YEAR_85_TEXTURE_INDEX 1
#define YEAR_96_TEXTURE_INDEX 2
#define YEAR_06_TEXTURE_INDEX 3
#define YEAR_10_TEXTURE_INDEX 4
#define YEAR_15_TEXTURE_INDEX 5
#define YEAR_20_TEXTURE_INDEX 6
#define YEAR_25_TEXTURE_INDEX 7
#define YEAR_30_TEXTURE_INDEX 8

#define POP_76_TEXTURE_INDEX 9
#define POP_85_TEXTURE_INDEX 10
#define POP_96_TEXTURE_INDEX 11
#define POP_06_TEXTURE_INDEX 12

#define FOOT_76_TEXTURE_INDEX 13
#define FOOT_85_TEXTURE_INDEX 14
#define FOOT_96_TEXTURE_INDEX 15
#define FOOT_06_TEXTURE_INDEX 16

#define SLOPE_TEXTURE_INDEX 17
#define ROAD_DENSITY_TEXTURE_INDEX 18
#define DISTANCE_TO_URBAN_CENTER_TEXTURE_INDEX 19
#define DISTANCE_TO_INTERCHANGE_TEXTURE_INDEX 20

#define AGE_TEXTURE_INDEX 21
#define INCOME_TEXTURE_INDEX 22

#define DEVP_06_TEXTURE_INDEX 24

#define CITY_SELECTION_LINE_WIDTH 1.5
#define COUNTY_SELECTION_LINE_WIDTH 3.0
#define HOUSE_DISTRICTS_SELECTION_LINE_WIDTH 2.0


#define ACTION_BOX_LINE_COLOR 0.05,0.2,0.4
#define ACTION_BOX_COLOR 0.53,0.60,0.78
#define ACTION_BOX_COLOR_2 0.53,0.60,0.78
#define ACTION_BOX_COLOR_OFF 0.38, 0.45, 0.63 //0.43,0.50,0.68
#define ACTION_BOX_COLOR_EXTRA 0.68,0.75,0.93 //0.63,0.70,0.88
#define ACTION_BOX_COLOR_EXTRA2 0.83,0.90,1.0 //0.63,0.70,0.88
#define ACTION_BOX_BORDER_SIZE 1
#define ACTION_BOX_SIZE 30
#define ACTION_BOX_SPACING 1
//#define VIS_MODE_BOX_WIDTH 250


//new map colors
#define PROTECTED_R 59
#define PROTECTED_G 117
#define PROTECTED_B 58
#define WATER_R 4
#define WATER_G 90
#define WATER_B 227
#define DEVELOPED_R 165
#define DEVELOPED_G 205
#define DEVELOPED_B 159
#define UNDEVELOPED_R 79
#define UNDEVELOPED_G 136
#define UNDEVELOPED_B 91

#define FPS_LABEL_COLOR 0,0,0.5
#define FPS_LABEL_LINE_WIDTH 1

#define STROKE_CHARACTER_SPACING 10

#define PANE_BORDER_COLOR 0.18, 0.25, 0.35 //0.26, 0.33, 0.43   //0.50,0.56,0.67
#define PANE_COLOR  0.47,0.54,0.65    //0.68,0.77,.91
#define PANE_BORDER 1

#define PAD_VIS_FROM_TOP 35
#define PROBE_INTERFACE_ROI_COLOR_BOX_HEIGHT 4

#define PROBE_INTERFACE_MODE_LABEL_LINE_WIDTH 1
#define PROBE_INTERFACE_MODE_LABEL_SCALE 0.13 //0.11
#define PROBE_INTERFACE_MODE_LABEL_OFFSET 21 //0.11
#define PROBE_INTERFACE_MODE_LABEL_COLOR 0,0,0

#define SELECTION_IN_PROGRESS_LINE_COLOR 1.0,0.0,0.0
#define SELECTION_IN_PROGRESS_LINE_WIDTH 2.0

#define DEFAULT_PROBE_INTERFACE_WIDTH 500
#define DEFAULT_PROBE_INTERFACE_HEIGHT 300

#define DEFAULT_COMPARISON_INTERFACE_WIDTH 750
#define DEFAULT_COMPARISON_INTERFACE_HEIGHT 450

#define PROBE_INTERFACE_PLACEMENT_DISTANCE 350
#define PROBE_INTERFACE_PLACEMENT_SCREEN_EDGE_BUFFER 10

#define ROI_INNER_LINE_COLOR_1	1,0.5,0		//orange
#define ROI_INNER_LINE_COLOR_2	1,0,1		//magenta
#define ROI_INNER_LINE_COLOR_3	0,1,0		//neon green
#define ROI_INNER_LINE_COLOR_4	0,1,1		//cyan
#define ROI_INNER_LINE_COLOR_5	.62,.32,1   //violet
#define ROI_INNER_LINE_COLOR_6	0.0,0.0,1	//blue
#define ROI_INNER_LINE_COLOR_7	1,.5,.5		//pink
#define ROI_INNER_LINE_COLOR_8	1,1,1		//white		
#define ROI_INNER_LINE_COLOR_9 1,0,0.5		//fuscia
#define ROI_INNER_LINE_COLOR_10	.72,.65,0	//golden
//#define ROI_INNER_LINE_WIDTH 2
//#define ROI_OUTER_LINE_COLOR 0,0,0
//#define ROI_OUTER_LINE_WIDTH 4

//video mod
#define ROI_INNER_LINE_WIDTH 3
#define ROI_OUTER_LINE_COLOR 0,0,0
#define ROI_OUTER_LINE_WIDTH 5


#define CONNECTOR_INNER_LINE_COLOR 1,1,0
#define CONNECTOR_INNER_LINE_WIDTH 2
#define CONNECTOR_OUTER_LINE_COLOR 0,0,0
#define CONNECTOR_OUTER_LINE_WIDTH 4

#define NUMBER_OF_ROI_CONNECTION_SITES 40  //this determines the max number of candidate locations around a ROI to check for possible connection making, more gives visually better results, less makes it faster

#define COUNTY_BORDER_COLOR 0,0,0
#define COUNTY_BORDER_COLOR_ALT 1,1,1
#define COUNTY_BORDER_LINE_WIDTH 1.5

#define COUNTY_LABEL_INNER_COLOR 1,1,1
#define COUNTY_LABEL_INNER_COLOR_ALT 1,1,1
#define COUNTY_LABEL_OUTER_COLOR 0,0,0
#define COUNTY_LABEL_OUTER_COLOR_ALT 1,1,1
#define COUNTY_LABEL_INNER_LINE_WIDTH 1.5
#define COUNTY_LABEL_OUTER_LINE_WIDTH 3.5
#define COUNTY_LABEL_LEFT_OFFSET 0.04 //this the ratio of total map size to offset size (to move left from center of county)
#define COUNTY_LABEL_SCALE_FACTOR .00012658228 //this the ratio of total map size to letter size

#define HOUSE_DISTRICTS_LABEL_INNER_COLOR 0.6,0.6,1.0
#define HOUSE_DISTRICTS_LABEL_OUTER_COLOR 0,0,0
#define HOUSE_DISTRICTS_LABEL_OUTER_COLOR 0,0,0
#define HOUSE_DISTRICTS_LABEL_INNER_LINE_WIDTH 1.0
#define HOUSE_DISTRICTS_LABEL_OUTER_LINE_WIDTH 3.5
#define HOUSE_DISTRICTS_LABEL_LEFT_OFFSET 0.02 //this the ratio of total map size to offset size (to move left from center of county)
#define HOUSE_DISTRICTS_LABEL_SCALE_FACTOR .00008658228 //this the ratio of total map size to letter size


//#define CITY_BOUNDS_LABEL_INNER_COLOR 0.75,0.75,0.75
#define CITY_BOUNDS_LABEL_INNER_COLOR 0.0,0.0,0.0
#define CITY_BOUNDS_LABEL_INNER_COLOR_ALT 1.0,1.0,1.0
#define CITY_BOUNDS_LABEL_OUTER_COLOR 0,0,0
//#define CITY_BOUNDS_LABEL_INNER_LINE_WIDTH 1.5
#define CITY_BOUNDS_LABEL_INNER_LINE_WIDTH .75
#define CITY_BOUNDS_LABEL_OUTER_LINE_WIDTH 3.5
#define CITY_BOUNDS_LABEL_LEFT_OFFSET 0.000 //this the ratio of total map size to offset size (to move left from center of the city boundary)
#define CITY_BOUNDS_LABEL_SCALE_FACTOR .00003658228 //this the ratio of total map size to letter size

#define CITY_BORDER_COLOR 0.10,0.10,0.10
#define CITY_BORDER_COLOR_ALT 0.90,0.90,0.90
#define CITY_BORDER_LINE_WIDTH 0.5

#define HOUSE_DISTRICTS_BORDER_COLOR 0.0,0.0,0.15
#define HOUSE_DISTRICTS_BORDER_COLOR_ALT 1.0,1.0,0.85
#define HOUSE_DISTRICTS_BORDER_LINE_WIDTH 1.5


#define INTERSTATES_COLOR 0.75,0,0
#define INTERSTATES_LINE_WIDTH 1.0

#define PRIMARY_ROADS_COLOR 0.20,0.20,0.20
#define PRIMARY_ROADS_COLOR_ALT 0.80,0.80,0.80
#define PRIMARY_ROADS_LINE_WIDTH 1.0

#define ALL_ROADS_COLOR 0.3,0.3,0.3
#define ALL_ROADS_COLOR_ALT 0.7,0.7,0.7
#define ALL_ROADS_LINE_WIDTH 0.5

#define CITY_CENTER_COLOR 0.50,0,0
#define CITY_CENTER_POINT_SIZE 4.5

#define LEGEND_BOX_SIZE 20
#define LEGEND_TEXT_SIZE 0.12
#define LEGEND_TEXT_LINE_WIDTH 1
#define LEGEND_INSET 10
#define LEGEND_TEXT_COLOR .85,.85,1

#define PIE_GRAPH_TEXT_SIZE 0.075
#define PIE_GRAPH_TEXT_LINE_WIDTH 1
#define PIE_GRAPH_TEXT_COLOR 1,1,1


#define TOUCH_BUTTON_SIZE 125
#define MOUSE_BUTTON_SIZE 95
#define LOWRES_BUTTON_SIZE 75
#define YEAR_LABEL_HEIGHT 300
#define YEAR_LABEL_WIDTH  150

#define MODE_BUTTON_COLOR_0 0.5, 0.5, 0.5
#define MODE_BUTTON_COLOR_1 0.75, 0.75, 0.75
#define MODE_BUTTON_COLOR_2 1,1,1

#define ACTIVE_BUTTON_COLOR 1.0, 1.0, 0.0, 1.0
#define ACTIVE_BUTTON_COLOR_2 1.0, 0.0, 1.0, 1.0
#define ACTIVE_BUTTON_COLOR_3 0.0, 1.0, 1.0, 1.0


#define USE_TIME_SCALE_LABEL_COLOR 1,1,1
#define USE_TIME_SCALE_LABEL_WIDTH 1
#define USE_TIME_SCALE_LABEL_SCALE 0.075
#define USE_TIME_SCALE_LABEL_OFFSET 28
#define USE_TIME_SCALE_LABEL_SHIFT 8

#define USE_TIME_SCALE_LINE_COLOR 1.0,1.0,1.0
#define USE_TIME_SCALE_LINE_WIDTH 0.5

#define USE_TIME_YEAR_LINE_COLOR 1,1,1
#define USE_TIME_YEAR_LINE_WIDTH 0.5
#define USE_TIME_YEAR_LABEL_COLOR 1,1,1
#define USE_TIME_YEAR_LABEL_WIDTH 1
#define USE_TIME_YEAR_LABEL_SCALE 0.075
#define USE_OVER_TIME_LABEL_SHIFT 25
#define USE_TIME_YEAR_BOTTOM_LABEL_OFFSET 13




#define MAUP_LABEL_COLOR 1,1,1
#define MAUP_LABEL_WIDTH 1
#define MAUP_LABEL_SCALE 0.075


#define GROWTH_TIME_SCALE_LABEL_OFFSET 48


///ABOVE DEF USED IN URBAN GROWTH PROBE VIS
///BELOW LEFTOVERS FROM AFGHANISTAN PROBE VIS?

#define CONTENT_INSET 8
#define CONTENT_INSET_TOP 54
#define PCOORD_LABEL_INSET 10

#define MAP_HIGHLIGHT_COLOR 1.0,1.0,0.5
#define MAP_HIGHLIGHT_ALPHA 0.25
#define MAP_LABEL_COLOR .85,.85,.85
#define MAP_ACTION_BOX_LINE_WIDTH 1.0

#define LABEL_INSET 8
#define LABEL_COLOR 0,0,0

#define BUTTON_INSET 30
#define BUTTON_INSET_2 48
#define BUTTON_HEIGHT 14
#define BUTTON_COLOR 0.53,0.60,0.78  //0.2,0.5,0.9
#define BUTTON_LINE_COLOR 0.05,0.2,0.4
#define BUTTON_LINE_WIDTH 1 //0.5
#define SELECTED_BUTTON_COLOR 0.66,0.75,.89 //0.3,0.6,1.0
#define SELECTED_BUTTON_LINE_WIDTH 1.5

#define SLIDER_BORDER_COLOR 0.025,0.1,0.2
#define SLIDER_SLIDE_COLOR 0.05,0.2,0.4
#define SLIDER_WIDTH 16
#define SLIDER_KNOB_WIDTH 20
#define SLIDER_KNOB_HEIGHT 16
#define SLIDER_LABEL_OFFSET 25
#define SLIDER_KNOB_COLOR 1.0,0.8,0.1
#define SLIDER_KNOB_BORDER_COLOR 0.8,0.6,0.0
#define SLIDER_KNOB_BORDER_WIDTH 0.5
#define SLIDER_BORDER_WIDTH 1.5
#define SLIDER_DOUBLE_LABEL_INSET 26
#define SLIDER_VALUE_LABEL_COLOR 1.0,0.8,0.1

//#define PANE_BORDER_COLOR 0.05,0.2,0.4
//#define PANE_COLOR       0.1,0.4,0.8



#define LINE_COLOR       0.0,0.0,0.0
#define AXIS_COLOR       0.15,0.15,0.15

#define NORMAL_PROBE_COLOR 0.8,0.8,0
#define HIGHLIGHTED_PROBE_COLOR 1,1,0
#define PROBE_OUTER_COLOR 0,0,0
#define PROBE_OUTER_SIZE 3
#define PROBE_INNER_SIZE 1
#define PROBE_HIGHLIGHTED_INNER_SIZE 2

//error color to show up whenever data is bad or parsed incorrectly
#define ERROR_COLOR 0,0,0


#define COMPARISON_SELECTION_MODE_DARKENING .5
#define COMPARISON_USAGE_GRAPH_LINE_WIDTH 4
#define COMPARISON_USAGE_GRAPH_STIPPLE_LABEL_LINE_WIDTH 1.75
#define COMPARISON_USAGE_GRAPH_STIPPLE_LABEL_PATTERN 0x0707


#define DEFAULT_MAUP_INTERFACE_HEIGHT 200

*/