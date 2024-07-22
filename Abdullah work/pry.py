import os

def print_env_variables():
    # Print all environment variables
    for key, value in os.environ.items():
        print(f"{key}={value}")

def write_message_to_file():
    # Write a message to a file
    with open("message.txt", "w") as file:
        file.write("This is a message written to the file.")

if __name__ == "__main__":
    print_env_variables()
    write_message_to_file()
