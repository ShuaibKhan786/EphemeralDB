# First Stage: Build the application
FROM gcc:4.9 AS builder

WORKDIR /home

# Copy the source code and Makefile into the container
COPY src/ ./src/
COPY Makefile ./Makefile

# Build the application using the Makefile
RUN make all

# Second Stage: Create a lightweight runtime container
FROM debian:buster-slim

WORKDIR /home

# Copy the compiled binary from the builder stage
COPY --from=builder /home/ephemeraldb /home/bin/ephemeraldb

# Set the default command to run the application
CMD ["./bin/ephemeraldb"]
