package gomock

import (
	"github.com/golang/mock/gomock"

	// "gomock/spider"

	mock_spider "gomock/spider/mock"

	"testing"
)

func TestGetGoVersion(t *testing.T) {
	// 此时 CreateGoVersionSpider() 可能还没有实现，或者在单元测试环境下不能运行
	// 这里也可以是连接数据库的相关操作
	// 可以使用gomock来Mock一个实例
    // v := GetGoVersion(spider.CreateGoVersionSpider())

	ctrl := gomock.NewController(t)
	defer ctrl.Finish()

	// 通过 Mock 工具 模拟出来一个 Spider,本来Spider中有个不能调用的函数,通过Mock之后就可以给不能调用的函数赋期望的值
	mockSpider := mock_spider.NewMockSpider(ctrl)
	// 这里搬来是 通过 调用 GetGoVersion并传入一个不能测试的函数,以此返回go版本的
	// 这里Mock的Spider，其实就是一个spider.Spider类型的值,只不过设置了相关返回值而已
	mockSpider.EXPECT().GetBody().Return("go1.8.3")
	// 通过 Mock 直接不需要调用 GetGoVersion函数,直接按照预期返回就行
	// GetGoVersion会调用mockSpider的GetBody函数
	goVer := GetGoVersion(mockSpider)

    if goVer != "go1.8.3" {
        t.Errorf("Get wrong version %s", goVer)
    }
}