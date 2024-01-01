#include <mbed.h>             // Include Mbed framework for ARM microcontrollers
#include "I3G4250D.h"         // Include driver for I3G4250D gyroscope
#include "LCD_DISCO_F429ZI.h" // Include driver for LCD_DISCO_F429ZI display

/* Global variables */
static mbed::BufferedSerial serial_port(USBTX, USBRX); // Serial port for communication (e.g., with a PC)

// Arrays to store gyroscopic data, calculated velocities, and distances
float samples[40][3];       // 40 samples, each with 3 axis readings
float velo_samples[40][3];  // 40 samples of calculated velocities on 3 axes
float distance_samples[40]; // 40 samples of calculated distances
double global_distance = 0; // Total calculated distance

/* Gyroscope radius constants */
#define X 1
#define Y 1
#define Z 0.548

/* Threshold constants for gyro data */
#define MAX_THRESH 500
#define MIN_THRESH -500

/* Function prototypes */
void ProcessXYZ(float *gyro_xyz, float *velo_xyz, int half_second_count);
void DisplayData(LCD_DISCO_F429ZI &lcd, float *gyro_xyz, float *velo_xyz, int half_second_count);
void DisplayDistance(LCD_DISCO_F429ZI &lcd);
void DrawLineChart(LCD_DISCO_F429ZI &lcd, float *data, int data_length);
void ClearScreen();

int main()
{
    // Main function: sets up peripherals and contains the main loop

    serial_port.set_baud(9600); // Set baud rate for serial communication
    SPI spi(PF_9, PF_8, PF_7);  // SPI interface setup
    DigitalOut CS(PC_1);        // Chip Select for SPI
    DigitalIn BUTTON(PA_0);     // Button input for user interaction

    /* Configure SPI */
    CS.write(1);            // Set Chip Select high
    spi.format(8, 3);       // Set SPI format: 8 bits per frame, Mode 3
    spi.frequency(1000000); // Set SPI frequency

    // Variables for gyroscope ID, and loop counter
    int gyro_id = 0;
    int half_second_count = 0;

    // Arrays to store gyroscope and velocity data
    float gyro_xyz[3]; // Store angular velocity
    float velo_xyz[3]; // Store calculated velocity

    gyro_id = Init(spi, CS); // Initialize gyroscope

    /* LCD Initialization */
    LCD_DISCO_F429ZI lcd;                   // Create LCD object
    int screen_height = BSP_LCD_GetYSize(); // Get LCD screen height

    while (true)
    {
        bool stay = true;
        // Wait for user to press button
        ClearScreen(); // Clear the LCD screen
        // Display instructions on the LCD
        BSP_LCD_SetFont(&Font20); // Set font size to 20
        lcd.DisplayStringAt(0, screen_height / 2 - 150, (uint8_t *)"Press the blue", LEFT_MODE);
        lcd.DisplayStringAt(0, screen_height / 2 - 120, (uint8_t *)"button to start.", LEFT_MODE);
        while (!BUTTON.read())
            ; // Wait until the button is pressed

        // Initialize screen with different sections for displaying data
        ClearScreen();            // Clear the screen for new data display
        BSP_LCD_SetFont(&Font20); // Set font size to 20
        // Display headers for different data sections
        lcd.DisplayStringAt(0, screen_height / 2 - 150, (uint8_t *)"Gyro values", CENTER_MODE);
        lcd.DisplayStringAt(0, screen_height / 2 - 20, (uint8_t *)"Linear Velocity", CENTER_MODE);
        lcd.DisplayStringAt(0, screen_height / 2 + 110, (uint8_t *)"Time passed", CENTER_MODE);
        ThisThread::sleep_for(1000); // Pause for a second

        while (stay)
        {
            ReadXYZ(spi, CS, gyro_xyz);                        // Read gyro data
            ProcessXYZ(gyro_xyz, velo_xyz, half_second_count); // Process the data
            if (half_second_count < 40)
            {
                // If less than 20 seconds have passed, display current data
                DisplayData(lcd, gyro_xyz, velo_xyz, half_second_count);
            }
            else if (half_second_count == 40)
            {
                // At 20 seconds, display the total distance
                DisplayDistance(lcd);
                half_second_count++;
            }
            else
            {
                // After 20 seconds, check if the button is pressed to restart
                if (BUTTON.read())
                {
                    stay = false;
                    half_second_count = 0;
                    global_distance = 0;
                }
            }
            half_second_count++;
            if (half_second_count <= 40)
                ThisThread::sleep_for(500); // Wait half a second between readings
            else
                ThisThread::sleep_for(3); // Shorter delay after 20 seconds
        }
    }
}
// ProcessXYZ function implementation
void ProcessXYZ(float *gyro_xyz, float *velo_xyz, int half_second_count)
{
    // Processes the raw gyroscopic data to calculate linear velocity

    // Check if gyro data is within the defined thresholds
    for (int i = 0; i < 3; i++)
    {
        if (gyro_xyz[i] > MAX_THRESH || gyro_xyz[i] < MIN_THRESH)
        {
            gyro_xyz[i] = 0; // Set out-of-range values to zero
        }
    }

    // Calculate linear velocity
    if (half_second_count == 0)
    {
        // If this is the first reading, set initial velocity to zero
        velo_xyz[0] = 0;
        velo_xyz[1] = 0;
        velo_xyz[2] = 0;
    }
    else
    {
        // Calculate linear velocity from angular velocity and radius
        // Linear velocity = angular velocity * radius
        velo_xyz[0] = (samples[half_second_count - 1][0] - gyro_xyz[0]) * X;
        velo_xyz[1] = (samples[half_second_count - 1][1] - gyro_xyz[1]) * Y;
        velo_xyz[2] = (samples[half_second_count - 1][2] - gyro_xyz[2]) * Z;
    }
}

// DisplayData function implementation
void DisplayData(LCD_DISCO_F429ZI &lcd, float *gyro_xyz, float *velo_xyz, int half_second_count)
{
    // Displays the current gyroscopic data and calculated velocity on the LCD screen

    // Variables for formatting the display strings
    char gyro_x[30] = {0};
    char gyro_y[30] = {0};
    char gyro_z[30] = {0};

    char x_velo[30] = {0};
    char y_velo[30] = {0};
    char z_velo[30] = {0};

    char time_display[15] = {0};

    // Accumulate data until 20 seconds
    for (int i = 0; i < 3; i++)
    {
        samples[half_second_count][i] = gyro_xyz[i] = gyro_xyz[i];
        velo_samples[half_second_count][i] = velo_xyz[i];
    }

    // Format and display gyroscopic data on the LCD
    sprintf(gyro_x, "X Raw_S: %5.2f", gyro_xyz[0]); // Format X-axis gyro data
    sprintf(gyro_y, "Y Raw_S: %5.2f", gyro_xyz[1]); // Format Y-axis gyro data
    sprintf(gyro_z, "Z Raw_S: %5.2f", gyro_xyz[2]); // Format Z-axis gyro data

    // Format and display velocity data on the LCD
    sprintf(x_velo, "X Lin_S: %5.2f", velo_xyz[0]); // Format X-axis velocity
    sprintf(y_velo, "Y Lin_S: %5.2f", velo_xyz[1]); // Format Y-axis velocity
    sprintf(z_velo, "Z Lin_S: %5.2f", velo_xyz[2]); // Format Z-axis velocity

    // Display time passed since the start of measurements
    sprintf(time_display, "%5.2f s", half_second_count * 0.5); // Calculate and format time

    // Get screen height for positioning the text
    int screen_height = BSP_LCD_GetYSize();

    // Display the formatted strings on the LCD
    lcd.DisplayStringAt(0, screen_height / 2 - 120, (uint8_t *)gyro_x, LEFT_MODE);
    lcd.DisplayStringAt(0, screen_height / 2 - 90, (uint8_t *)gyro_y, LEFT_MODE);
    lcd.DisplayStringAt(0, screen_height / 2 - 60, (uint8_t *)gyro_z, LEFT_MODE);

    lcd.DisplayStringAt(0, screen_height / 2 + 10, (uint8_t *)x_velo, LEFT_MODE);
    lcd.DisplayStringAt(0, screen_height / 2 + 40, (uint8_t *)y_velo, LEFT_MODE);
    lcd.DisplayStringAt(0, screen_height / 2 + 70, (uint8_t *)z_velo, LEFT_MODE);

    // Display the calculated time
    lcd.DisplayStringAt(0, screen_height / 2 + 140, (uint8_t *)time_display, CENTER_MODE);
}

// DisplayDistance function implementation
void DisplayDistance(LCD_DISCO_F429ZI &lcd)
{
    // Calculate and display the total distance based on velocity data

    char distance_display[25]; // String to display the distance
    // Iterate over each velocity sample to calculate distance
    for (int i = 0; i < 40; i++)
    {
        // Distance calculation: velocity * time (0.5 seconds per sample)
        float x_dist = velo_samples[i][0] * 0.5;
        float y_dist = velo_samples[i][1] * 0.5;
        float z_dist = velo_samples[i][2] * 0.5;

        // Calculate the distance for this sample
        distance_samples[i] = sqrt(x_dist * x_dist);
        global_distance += distance_samples[i]; // Add to total distance
    }

    // Calibration and display of total distance
    int screen_height = BSP_LCD_GetYSize();
    DrawLineChart(lcd, distance_samples, 40); // Draw a line chart of distance over time

    // Set font and display text for the distance section
    BSP_LCD_SetFont(&Font24);
    lcd.DisplayStringAt(0, screen_height / 2 + 40, (uint8_t *)"Distance", CENTER_MODE);

    // Format and display the total distance
    sprintf(distance_display, "%5.2f m", global_distance);
    lcd.DisplayStringAt(0, screen_height / 2 + 60, (uint8_t *)distance_display, CENTER_MODE);

    // Display instructions for restarting the measurement
    lcd.DisplayStringAt(0, screen_height / 2 + 100, (uint8_t *)"Press again", LEFT_MODE);
    lcd.DisplayStringAt(0, screen_height / 2 + 140, (uint8_t *)"to restart.", LEFT_MODE);

    // Perform a calibration offset and scaling on the total distance
    global_distance -= 0.035; // Offset adjustment
    if (global_distance < 0)
    {
        global_distance = 0; // Ensure distance doesn't go negative
    }
    global_distance /= 0.165; // Scaling factor
}

// DrawLineChart function implementation
void DrawLineChart(LCD_DISCO_F429ZI &lcd, float *data, int data_length)
{
    // Draws a line chart on the LCD based on the provided data

    ClearScreen(); // Clear the LCD screen before drawing the chart

    // Determine the maximum Y value for the chart scale
    float maxYValue = data[0] * 100;
    for (int i = 1; i < data_length; i++)
    {
        if (data[i] * 100 > maxYValue)
        {
            maxYValue = data[i] * 100; // Update maxYValue if current data point is larger
        }
    }

    // Draw the axes for the line chart
    int screen_height = BSP_LCD_GetYSize();
    BSP_LCD_DrawLine(10, 10, 10, screen_height / 2 - 10);                                          // Draw Y-axis
    BSP_LCD_DrawLine(10, screen_height / 2 - 10, BSP_LCD_GetXSize() - 10, screen_height / 2 - 10); // Draw X-axis

    // Set font for axis labels
    BSP_LCD_SetFont(&Font16);
    // Display axis labels
    BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize() - 90, screen_height / 2 + 10, (uint8_t *)"x (0.5s)", LEFT_MODE);
    BSP_LCD_DisplayStringAt(40, 0, (uint8_t *)"y (cm)", LEFT_MODE);

    // Draw tick marks and labels on X-axis
    for (int i = 0; i <= 40; i += 5)
    {
        int x = 10 + (i * (BSP_LCD_GetXSize() - 20) / 40);                     // Calculate X position for tick mark
        BSP_LCD_DrawLine(x, screen_height / 2 - 10, x, screen_height / 2 - 5); // Draw tick mark

        char str[10];
        sprintf(str, "%d", i);                                                             // Format tick label
        BSP_LCD_DisplayStringAt(x - 10, screen_height / 2 - 5, (uint8_t *)str, LEFT_MODE); // Display label
    }

    // Draw line chart based on data
    if (maxYValue >= 1)
    {
        int numYTicks = 5; // Number of Y-axis tick marks
        for (int i = 1; i <= numYTicks; ++i)
        {
            float yValue = (maxYValue / numYTicks) * i;
            int y = screen_height / 2 - 10 - (int)((yValue / maxYValue) * (screen_height / 2 - 20));

            BSP_LCD_DrawLine(5, y, 10, y); // Draw Y-axis tick mark

            char str[10];
            sprintf(str, "%.0f", yValue);                                   // Format Y-axis tick label
            BSP_LCD_DisplayStringAt(12, y - 10, (uint8_t *)str, LEFT_MODE); // Display label
        }
        // Draw the actual line chart
        for (int i = 0; i < data_length - 1; i++)
        {
            int x1 = 10 + (i * (BSP_LCD_GetXSize() - 20) / 40);
            int y1 = screen_height / 2 - 10 - (int)((data[i] * 100 / maxYValue) * (screen_height / 2 - 20));

            int x2 = 10 + ((i + 1) * (BSP_LCD_GetXSize() - 20) / 40);
            int y2 = screen_height / 2 - 10 - (int)((data[i + 1] * 100 / maxYValue) * (screen_height / 2 - 20));

            BSP_LCD_DrawLine(x1, y1, x2, y2); // Draw line segment
        }
    }
}

void ClearScreen()
{
    // Clears the LCD screen to prepare for new content

    BSP_LCD_Clear(LCD_COLOR_WHITE);        // Set screen background to white
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE); // Set background color for text
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK); // Set text color to black
}