# Secondary Indexes (Member 4)

Files: `secondaryIndex.h/.cpp` (one generic index), `secondaryIndexManager.h/.cpp` (the 3 required indexes + interface for the team), `Tests/testSecondaryIndex.cpp`.

Build the test (from the project root): `g++ -std=c++17 Tests/testSecondaryIndex.cpp SecondaryIndex/*.cpp -o testSecondaryIndex`

## How it works (linked list on a separate file)

| Index | Secondary key | Linked values |
|---|---|---|
| Doctor Name | `char[30]` | Doctor IDs |
| Doctor Specialty | `char[20]` | Doctor IDs |
| Appointment -> Doctor ID | `char[15]` | Appointment IDs |

Each index = 2 files in `Data/`:
* `*.idx` - `[int count]` then sorted unique keys `{char key[size]; int head}`. Searched with binary search.
* `*.lst` - `[int availHead]` then nodes `{char primaryKey[15]; int next}` (-1 ends the list).

Nodes store the **primary key**, not a byte offset, so the secondary index is bound to the primary index.
Lists are sorted by primary key. Deleted nodes get a `*` marker and go on the list file's own AVAIL LIST; inserts reuse them first.

## Interface for the other members

Create one `SecondaryIndexManager idx;` in main, call `idx.open()` at startup, then call the hook
**after** the data file + primary index operation succeeded:

| Operation | Hook |
|---|---|
| Add doctor | `onDoctorAdded(id, name, specialty)` |
| Delete doctor | `onDoctorDeleted(id, name, specialty)` |
| Update doctor name | `onDoctorNameUpdated(id, oldName, newName)` |
| Update specialty | `onDoctorSpecialtyUpdated(id, oldSpec, newSpec)` |
| Update Doctor ID | `onDoctorIDUpdated(oldID, newID, name, specialty)` |
| Add appointment | `onAppointmentAdded(apptID, doctorID)` |
| Delete appointment | `onAppointmentDeleted(apptID, doctorID)` |

Lookups return sorted primary keys (Member 3 maps them to offsets, Member 5 reads the records):
`doctorIDsByName`, `doctorIDsBySpecialty`, `appointmentIDsByDoctor`, `doctorIDsByNameAndSpecialty` (intersection, for the `... and ...` query).
`rebuild(doctors, appointments)` recreates all indexes from live records (use it if the index files are missing).

## Things the team should know

1. **Case-sensitive includes.** `doctor.cpp` includes `"Doctor.h"` and `doctorStorage.cpp` includes `"DoctorStorage.h"`, but the files are lower-case. Works on Windows, fails on Linux/macOS (and possibly on the TA's machine).
2. **Appointments.dat is never created** (no header, no `initialize`). `addAppointment` fails on a fresh machine.
3. **Appointment field order** is `date, id, doctorID` in `Appointment.h`, but the spec says ID first, and `*` should replace the first byte of the ID (currently offset 30).
4. **No AVAIL LIST yet** in `addDoctor` / `deleteDoctor` / `addAppointment` / `deleteAppointment` (Member 5's `insertRecord` reads the next pointer at `avail + 1`, which only works if the delete code writes it there - agree on one layout).
5. **Doctor ID update** changes the key but appointments keep the old ID. The secondary index re-points them; the Appointments file needs the same change (or the update should be rejected when the doctor has appointments).
