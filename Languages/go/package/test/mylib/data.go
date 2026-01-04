package mylib

type data struct {
	a int
	b int
}

// 推荐写法
func NewData() *data {
	return &data{
		a: 1,
		b: 2,
	}
}

// 要创建一个 data 前提是 有一个 data 实例😂？ 不推荐
// func (d *data) NewData() *data {
// 	return &data{
// 		a: d.a + 1,
// 		b: d.b + 1,
// 	}
// }

func (d *data) test() {
	println(d.a, d.b)
}

// --- 也可以使用别名来实现,提升权限
type DataTmp = data

func (d *DataTmp) Setb(b int) {
	d.b = b
}

func (d *DataTmp) Test() {
	d.test()
}