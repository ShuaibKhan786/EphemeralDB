package loadenv

import (
	"bufio"
	"errors"
	"io/fs"
	"os"
	"path/filepath"
	"strings"
)

const (
	EnvFileExt = ".env"
	EnvFileNotContentErr = ".env file doesnt exist in the given directory"
)

func Load(dirPath string) error {
	fileSystem := os.DirFS(dirPath)
	dir, err := fs.ReadDir(fileSystem, ".")
	if err != nil {
		return err
	}

	onceFileFound := false

	for _,entry := range dir {
		if envFileFound(dirPath,entry) {
			onceFileFound = true
			openEnvfile(fileSystem, entry)
		}else {
			if !onceFileFound {
				err = errors.New(EnvFileNotContentErr)
			}
		}
	}
	return err
}

func envFileFound(dirPath string,entry fs.DirEntry) bool {
	fullPath := filepath.Join(dirPath,entry.Name())
	fileExt := filepath.Ext(fullPath)

	if strings.Compare(fileExt,EnvFileExt) == 0{
		return true
	}else {
		return false
	}
}

func openEnvfile(fileSystem fs.FS, entry fs.DirEntry) error {
	envFile, err := fileSystem.Open(entry.Name())
	if err != nil {
		return err
	}
	defer envFile.Close()

	return readEnvfile(envFile)
}

func readEnvfile(envFile fs.File) error {
	var err error
	scanner := bufio.NewScanner(envFile)

	for scanner.Scan() {
		err = readLine(scanner.Text())
	}
	return err
}

func readLine(text string) error {
	splitInLine := strings.Split(text, " ")

	for _ , envVar := range splitInLine {
		return parseEnvVar(envVar)
	}
	return nil
}

func parseEnvVar(envVar string) error {
	splitEnvVar := strings.Split(envVar,"=")
	if len(splitEnvVar) == 2 {
		return setEnvVar(splitEnvVar)
	}else {
		return errors.New("unsupported environment variable")
	}
}

func setEnvVar(parsedEnvVar []string) error {
	key := strings.Trim(parsedEnvVar[0]," ")
	value := strings.Trim(parsedEnvVar[1]," ")

	exist := os.Getenv(key)
	if exist == "" {
		return setEnv(key, value)
	}else if strings.EqualFold(exist,value) {
		return nil
	}
	return setEnv(key, value)
}

func setEnv(key, value string) error {
	err := os.Setenv(key, value)
	if err != nil {
		return err
	}
	return nil
}