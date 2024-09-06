ARG GO_VERSION="1.22"
ARG ALPINE_VERSION="alpine3.20"


FROM golang:${GO_VERSION}-${ALPINE_VERSION} AS build-stage

WORKDIR /home

COPY go.mod go.sum ./
RUN go mod download

COPY internal/. /home/internal
COPY src/. /home/src

RUN CGO_ENABLED=0 GOOS=linux GOARCH=amd64 go build -o server ./src/main.go


FROM scratch AS final-stage

WORKDIR /home/server

COPY --from=build-stage /etc/ssl/certs/ca-certificates.crt /etc/ssl/certs/

COPY --from=build-stage /home/server ./


CMD ["./server"]
