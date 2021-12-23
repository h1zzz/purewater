// MIT License Copyright (c) 2021, h1zzz

package util

import (
	"os"
)

// PathExists ...
func PathExists(path string) (bool, error) {
	_, err := os.Stat(path)
	if err != nil {
		if os.IsNotExist(err) {
			return false, nil
		}
		return false, err
	}
	return true, nil
}
