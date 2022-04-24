// MIT License Copyright (c) 2022, h1zzz

package util

func ValidPort(port int) bool {
	if port <= 0 || port >= 65535 {
		return false
	}
	return true
}
