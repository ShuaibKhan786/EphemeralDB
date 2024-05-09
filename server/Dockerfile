FROM golang:1.22

WORKDIR /usr/ephemeraldb-client

COPY . ./

RUN go mod download && go mod verify

WORKDIR /usr/ephemeraldb-client/src

RUN go build -v -o /usr/ephemeraldb-client/bin/main

EXPOSE 8090

CMD [ "/usr/ephemeraldb-client/bin/main" ]
