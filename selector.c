#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define VEHICLE_MASS 1500
#define GRAVITY 9.81
#define WHEEL_RADIUS 0.3
#define FINAL_DRIVE_RATIO 3.9
#define ENGINE_TORQUE 250

const float GEAR_RATIOS[] = {3.5, 2.1, 1.4, 1.0, 0.8, 0.6}; 

typedef struct {
    float speed;
    float rpm;
    float angle;
    float throttle;
    int mode;
} Vehicle;

float fuzzy_speed(float speed) {
    if (speed < 5) return 0.0;
    if (speed < 15) return (speed - 5) / 10.0;
    if (speed < 30) return 0.3 + (speed - 15) / 30.0;
    if (speed < 50) return 0.6 + (speed - 30) / 40.0;
    return 1.0;
}

float fuzzy_rpm(float rpm) {
    if (rpm < 500) return 0.0;
    if (rpm < 1200) return (rpm - 500) / 700.0;
    if (rpm < 2500) return 0.3 + (rpm - 1200) / 1300.0;
    if (rpm < 4000) return 0.6 + (rpm - 2500) / 1500.0;
    return 1.0;
}

float fuzzy_angle(float angle) {
    if (angle < -15) return 0.1;        // Steep downhill
    if (angle < -5) return 0.3;         // Moderate downhill
    if (angle < 5) return 0.5;          // Flat
    if (angle < 15) return 0.7;         // Moderate uphill
    return 0.9;                         // Steep uphill
}

float fuzzy_throttle(float throttle) {
    if (throttle < 5) return 0.05;
    if (throttle < 20) return 0.2 + (throttle - 5) / 15.0;
    if (throttle < 60) return 0.5 + (throttle - 20) / 80.0;
    return 1.0;
}

float calculate_wheel_torque(float rpm, int gear) {
    // This represents the torque actually delivered to the wheels
    return (ENGINE_TORQUE * GEAR_RATIOS[gear - 1] * FINAL_DRIVE_RATIO) / WHEEL_RADIUS;
}

float calculate_acceleration(float torque) {
    return torque / VEHICLE_MASS;
}

int determine_gear(Vehicle vehicle) {
    float speed_level = fuzzy_speed(vehicle.speed);
    float rpm_level = fuzzy_rpm(vehicle.rpm);
    float angle_level = fuzzy_angle(vehicle.angle);
    float throttle_level = fuzzy_throttle(vehicle.throttle);

    float best_torque = 0;
    int best_gear = 1;

    // Ensure very low speeds stay in lower gears, cause for some reason it doesn't like doing that
    if (vehicle.speed < 8) return 1;
    if (vehicle.speed < 20 && rpm_level < 0.4) return 2;

    for (int gear = 1; gear <= 6; gear++) {
        float torque = calculate_wheel_torque(vehicle.rpm, gear);
        float acceleration = calculate_acceleration(torque);

        if (acceleration > best_torque) {
            best_torque = acceleration;
            best_gear = gear;
        }
    }

    // Adjust gear based on mode, this sort of works, but not really
    if (vehicle.mode == 1) {    // Tow Mode
        if (rpm_level > 0.8 || speed_level > 0.85) return best_gear;
        if (rpm_level > 0.65 || speed_level > 0.7) return best_gear - 1;
        if (rpm_level > 0.5 || speed_level > 0.5) return best_gear - 2;
        if (rpm_level > 0.3 || speed_level > 0.35 || angle_level > 0.7) return best_gear - 3;
        if (angle_level > 0.8 || throttle_level > 0.7) return best_gear - 4;
        return 1;
    } else {                    // Eco Mode
        if (vehicle.speed > 40 && vehicle.rpm < 2200) return 3; // Speed 40+, RPM < 2200 -> 3rd gear
        if (vehicle.speed > 40 && vehicle.rpm < 2500) return 3; 

        if (vehicle.speed > 100) return 5;  // Highway speed -> higher gear, usually 5th
        if (rpm_level > 0.7 || speed_level > 0.85) return best_gear;
        if (rpm_level > 0.55 || speed_level > 0.7) return best_gear - 1;
        if (rpm_level > 0.35 || speed_level > 0.5) return best_gear - 2;
        if (rpm_level > 0.2 || speed_level > 0.3 || angle_level > 0.7) return best_gear - 3;
        if (angle_level > 0.8 || throttle_level > 0.7) return best_gear - 4;
        return 1;
    }
}

bool get_vehicle_input(Vehicle* vehicle) {
    printf("Enter vehicle speed (km/h): ");
    if (scanf("%f", &vehicle->speed) != 1 || vehicle->speed < 0) return false;

    printf("Enter engine RPM: ");
    if (scanf("%f", &vehicle->rpm) != 1 || vehicle->rpm < 0) return false;

    printf("Enter ground angle (degrees): ");
    if (scanf("%f", &vehicle->angle) != 1) return false;

    printf("Enter throttle percentage (0-100): ");
    if (scanf("%f", &vehicle->throttle) != 1 || vehicle->throttle < 0 || vehicle->throttle > 100) return false;

    printf("Select mode (0 for EcoDrive, 1 for Tow Mode): ");
    if (scanf("%d", &vehicle->mode) != 1 || (vehicle->mode != 0 && vehicle->mode != 1)) return false;

    return true;
}

int main() {
    Vehicle vehicle;

    if (!get_vehicle_input(&vehicle)) {
        printf("Try that again...\n");
        return 1;
    }

    int recommended_gear = determine_gear(vehicle);
    printf("Recommended Gear (maybe, probably, possibly): %d\n", recommended_gear);

    return 0;
}
