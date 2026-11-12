# Healthcare-Management-System
A C++ Healthcare Management System implementing binary file processing, fixed-length records, primary and secondary indexing, AVAIL LIST management, and a query engine for managing doctors and appointments.

## Run a query

Choose **Run Query** from the main menu. Queries support a single selected field or `*`, equality conditions, and multiple conditions joined with `AND`. Field and keyword capitalization is flexible.

```sql
SELECT * FROM Doctors WHERE Specialty = 'Cardiology' AND Name = 'Jane Smith'
SELECT AppointmentDate FROM Appointments WHERE DoctorID = 'D001'
SELECT * FROM Doctors
```

Doctor fields are `DoctorID`, `Name`/`DoctorName`, `Specialty`, and `Address`. Appointment fields are `AppointmentID`, `Date`/`AppointmentDate`, and `DoctorID`. Unindexed fields are searched by scanning the data file.
