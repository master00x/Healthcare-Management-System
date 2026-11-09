#ifndef Appointment_h
#define Appointment_h

struct Appointment
{
    char appointmentDate[30];
    char appointmentID[15];
    char DoctorID[15];
};

const int APPPOINTMENT_SIZE = 60;

static_assert(sizeof(Appointment) == APPPOINTMENT_SIZE, "Appointment must be 60 bytes in size");

#endif