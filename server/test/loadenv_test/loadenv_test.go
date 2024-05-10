package loadenv_test

import (
	"io"
	"os"
	"path/filepath"
	"reflect"
	"testing"
	"testing/fstest"

	loadenv "github.com/ShuaibKhan786/EphemeralDB-client/internal/loadEnv"
)

func TestLoadenv(t *testing.T) {

	const booContent  = `API_KEY=your-api-key
DB_HOST=localhost
DB_PORT=5432`

	tempDir := t.TempDir()

	fs := fstest.MapFS{
		"foo.txt": {Data: []byte("")},
		"loo.txt": {Data: []byte("")},
		"boo.env": {Data: []byte(booContent)},
	}

    for name := range fs {
        path := filepath.Join(tempDir, name)
        f, err := fs.Open(name)
        if err != nil {
            t.Fatal(err)
        }
        data, err := io.ReadAll(f)
        if err != nil {
            t.Fatal(err)
        } 
		if err := os.WriteFile(path, data, 0644); err != nil {
            t.Fatal(err)
    	}
	}
    
	t.Run("Testing for .env files contain true",func(t *testing.T) {
		err := loadenv.Load(tempDir)
		assertNoError(t,err)
	})

	t.Run("Testing env variable",func(t *testing.T) {
		var got []string
		got = append(got, os.Getenv("API_KEY"))
		got = append(got, os.Getenv("DB_HOST"))
		got = append(got, os.Getenv("DB_PORT"))

		want := []string{"your-api-key","localhost","5432"}
		
		if !reflect.DeepEqual(got,want) {
			t.Errorf("got : %v but want %v",got, want)
		}
	})
}

func assertNoError(t *testing.T,got error) {
	t.Helper()
	if got != nil {
		t.Errorf("doesnt want an error")
	}
}