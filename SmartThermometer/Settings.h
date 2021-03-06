
const int BEEP_FREQUENCY = 1488;

const int CARD_TIMEOUT_SEC = 8;
const int CARD_BEEP_MS = 100;

// Temperature Calibaration {Value, Real}
const double CAL_LOW[] = {33.1, 36.5};
const double CAL_MID[] = {33.4, 36.7};
const double CAL_HIGH[] = {33.8, 37.1};

const int BODY_TEMP_MEASURE_START_BEEP_MS = 20;
const int BODY_TEMP_MEASURE_STOP_BEEP_MS = 500;

// [TRIGGER!] <- WAIT_MS -> [Start measuring] <- MEASURE_BEGIN_MS -> [          Sampling.....       ]  [Done measuring]
//                                            <-                 MEASURE_END_MS                    -> 
const int BODY_TEMP_WAIT_MS = 200; // How long the person have to stay before the measure starts to prevent false trigger
const int BODY_TEMP_MEASURE_BEGIN_MS = 0;
const int BODY_TEMP_MEASURE_END_MS = 500;
const int BODY_TEMP_MIN_SAMPLES = 5;  // Keep sampling if there are too few samples

const double BODY_TEMP_REMEASURE_THEREHOLD_C = 0.4; // if MAX - MIN > Therehold then remeasue
const int BODY_TEMP_MAX_REMEASURE_TIMES = 10;

const int BODY_TEMP_INTERVAL_MS = 1500;
const int BODY_TEMP_STAY_SEC = 5;

const int ROOM_TEMP_UPDATE_SEC = 5;


// Strings
const String TAP_CARD_PROMPT = "Tap Your Card...";
const String MEASURING_PROMPT = "Measuring...";
const String MEASURE_FAILED_PROMPT = "Measure Failed!";
const String MEASURE_LEAVE_PROMPT = "Come Closer!";

const String UPLOAD_SUCCESS_PROMPT = "Uploaded!";
const String UPLOAD_FAILED_PROMPT = "Upload Failed!";
