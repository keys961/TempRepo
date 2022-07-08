package main

import (
	"fmt"
	"strings"
)

func main() {
	fmt.Printf("%v", lengthLongestPath("dir\n\tsubdir1\n\tsubdir2\n\t\tfile.ext"))
}

func lengthLongestPath(input string) int {
	markers := strings.Split(input, "\n")

	stack := make([]string, 0)
	curLength := 0
	maxLength := 0

	for _, m := range markers {
		d := depth(m)
		for len(stack) > d {
			originalDepth := len(stack) - 1
			curLength -= len(stack[originalDepth]) - originalDepth
			stack = stack[:len(stack)-1]
		}
		stack = append(stack, m)
		curLength += len(m) - d
		if isFile(m) {
			if maxLength < curLength+len(stack)-1 {
				maxLength = curLength + len(stack) - 1
			}
		}
	}

	return maxLength
}

func depth(marker string) int {
	d := 0
	for _, c := range marker {
		if c == '\t' {
			d += 1
		} else {
			break
		}
	}
	return d
}

func isFile(marker string) bool {
	return strings.Contains(marker, ".")
}
