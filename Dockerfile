# Use an official GCC image from the Docker Hub
FROM gcc:latest

# Set the working directory in the container
WORKDIR /usr/src/app

# Copy the current directory contents into the container at /usr/src/app
COPY . .

# Compile the C code
RUN gcc -o final_project main.c -lpthread

# Run the executable
CMD ["./final_project"]

