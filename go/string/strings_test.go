package testing

import (
	"bytes"
	"strings"
	"testing"
	"unsafe"
)

var S = strings.Repeat("a", 100)

func normalConv() bool {
	b := []byte(S)
	s2 := string(b)
	return s2 == S
}

func unsafeConv() bool {
	// 先转换成切片
	b := unsafe.Slice(unsafe.StringData(S), len(S))
	// 再装换成String
	s2 := unsafe.String(unsafe.SliceData(b), len(b))

	return s2 == S
}

func BenchmarkNormalConv(b *testing.B) {
	for i := 0; i < b.N; i++ {
		if !normalConv() {
			b.Fatal()
		}
	}
}

func BenchmarkUnsafeConv(b *testing.B) {
	for i := 0; i < b.N; i++ {
		if !unsafeConv() {
			b.Fatal()
		}
	}
}

// =========================================

const N = 1000
const C = "a"

var Str = strings.Repeat(C, N)

func concat() bool {
	var s2 string
	for i := 0; i < N; i++ {
		s2 += C
	}
	return s2 == Str
}

func join() bool {
	b := make([]string, N)
	for i := 0; i < N; i++ {
		b[i] = C
	}
	return strings.Join(b, "") == Str
}

func buffer() bool {
	var b bytes.Buffer
	b.Grow(N)
	for i := 0; i < N; i++ {
		b.WriteString(C)
	}
	return b.String() == Str
}

func BenchmarkConcat(b *testing.B) {
	for i := 0; i < b.N; i++ {
		if !concat() {
			b.Fatal()
		}
	}
}

func BenchmarkJoin(b *testing.B) {
	for i := 0; i < b.N; i++ {
		if !join() {
			b.Fatal()
		}
	}
}

func BenchmarkBuffer(b *testing.B) {
	for i := 0; i < b.N; i++ {
		if !buffer() {
			b.Fatal()
		}
	}
}
