import serial
import sys
import datetime
import os


def main(output_file_name):
    serial_port = "/dev/ttyUSB0"
    baud_rate = 9600

    with serial.Serial(serial_port, baud_rate, timeout=1) as ser:
        ser.flushInput()

        # Check if the output file is empty and needs a header
        write_header = (
            not os.path.exists(output_file_name)
            or not open(output_file_name, "r").readline()
        )

        with open(output_file_name, "a") as file:
            print("Collecting data. Press Ctrl+C to stop.\n")

            header_line = "Timestamp,Seconds since startup,TempA,TempB"
            if write_header:
                file.write(header_line + "\n")
            print(header_line)

            try:
                while True:
                    if ser.in_waiting:
                        line = ser.readline().decode("utf-8").strip()
                        timestamp = datetime.datetime.utcnow().strftime(
                            "%Y-%m-%d %H:%M:%S.%f"
                        )[:-3]
                        formatted_line = f"{timestamp},{line}"
                        print(formatted_line)
                        file.write(formatted_line + "\n")
            except KeyboardInterrupt:
                print("Data collection stopped by user.")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 logtemp.py output_file_name")
        sys.exit(1)
    main(sys.argv[1])
