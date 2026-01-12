package main

import (
	"bytes"
	"unsafe"
	"strings"
	"crypto/rand"
	"encoding/csv"
	"encoding/json"
	"encoding/base64"
	"encoding/binary"
	"log"
	"fmt"
)

func testBase64_1() {
	d := make([]byte, 10)
	rand.Read(d)

	s := base64.StdEncoding.EncodeToString(d)
	b, _ := base64.StdEncoding.DecodeString(s)

	if !bytes.Equal(d, b) {
		log.Fatal("not equal")
	}

	fmt.Println(s)
}

func testBase64_2() {
	b := bytes.NewBuffer(nil)

	w := base64.NewEncoder(base64.StdEncoding, b)
	w.Write([]byte("hello, world\n"))
	w.Write([]byte("hello, go"))
	// 记得关闭，base64编码是将3个字节转换成4个字符，所以需要计算解码后的长度
	// 调用Close告诉Encoder没有新数据，强制刷新
	w.Close()

	fmt.Println(b.String())
	// Decoder 返回类型没有 Close 方法，它本身就是只读不写的
	r := base64.NewDecoder(base64.StdEncoding, b)
	bs := make([]byte, base64.StdEncoding.DecodedLen(b.Len()))
	r.Read(bs)
	fmt.Println(string(bs))
}

func testBinary_1() {
	var x int64 = 0x11223344

	big := bytes.NewBuffer(nil)
	binary.Write(big, binary.BigEndian, x)

	little := bytes.NewBuffer(nil)
	binary.Write(little, binary.LittleEndian, x)

	fmt.Printf("B: %x\n", big.Bytes())
	fmt.Printf("L: %x\n", little.Bytes())
}

func testBinary_2() {
	x := 1
	p := (*byte)(unsafe.Pointer(&x))
	// true 小端， false 大端
	println(*p == 1)
}

func testCSV_1() {
	b := bytes.NewBuffer(nil)

	w := csv.NewWriter(b)
	w.Write([]string{"user1", "pass1", "1234567890", `data:"test"`})
	w.Write([]string{"user2", "pass2", "0987654321", `data:"test2"`})
	w.Flush()

	fmt.Println(b.String())

	r := csv.NewReader(b)
	records, err := r.ReadAll()
	if err != nil {
		log.Fatalln(err)
	}
	for _, record := range records {
		fmt.Println(strings.Join(record, " | "))
	}
}

func testJSON_1() {
	x := 100

	d := struct {
		X *int
		S string
		Y []int
	} {
		X: &x,
		S: "hello, world!",
		Y: []int{1, 2, 3, 4},
	}

	b, err := json.Marshal(d)
	if err != nil {
		log.Fatalln(err)
	}

	println(string(b))

	buf := bytes.NewBuffer(nil)
	json.Indent(buf, b, "", "  ")

	println(buf.String())
}

func testJSON_2() {
	x := 100

	d := struct {
		X *int		`json:"id"`
		S string	`json:"NAME"`
		Y []byte	`json:"data,omitempty"`
	}{
		X: &x,
		S: "hello, <world!>",
	}

	b, err := json.Marshal(d)
	if err != nil {
		log.Fatalln(err)
	}

	buf := bytes.NewBuffer(nil)
	json.Indent(buf, b, "", "  ")

	println(buf.String())
}

type U struct {
	Id int
	Name string
}

type M struct {
	U
	Title string
}

func main() {
	d := M { U{1, "user1"}, "title1"}

	b, err := json.Marshal(d)
	if err != nil {
		log.Fatalln(err)
	}

	println(string(b))

	func() {
		var x M

		if err := json.Unmarshal(b, &x); err != nil {
			log.Fatalln(err)
		}

		fmt.Printf("%+v\n", x)
	}()

	func() {
		// 不同类型
		// ID 必须是导出成员，后续字母大小写可被忽略
		var x struct {
			ID int
			Title string
		}

		if err := json.Unmarshal(b, &x); err != nil {
			log.Fatalln(err)
		}

		fmt.Printf("%+v\n", x)
	}()

	// 字典
	func() {
		var m map[string]interface{}

		if err := json.Unmarshal(b, &m); err != nil {
			log.Fatalln(err)
		}

		fmt.Printf("%+v\n", m)
	}()

	buf := bytes.NewBuffer(nil)
	json.Indent(buf, b, "", "  ")

	println(buf.String())
}