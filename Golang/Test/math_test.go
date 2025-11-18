package test

import (
	"fmt"
	"math"
	"math/rand"
	"testing"
	"time"

	"github.com/likexian/gokit/assert"
)

func TestAbs(t *testing.T) {
    tests := []struct {
        name  string
        input float64
        want  float64
    }{
        {"正数", 5.0, 5.0},
        {"负数", -5.0, 5.0},
        {"零", 0.0, 0.0},
        {"正小数", 3.14, 3.14},
        {"负小数", -3.14, 3.14},
        {"正无穷大", math.Inf(1), math.Inf(1)},
        {"负无穷大", math.Inf(-1), math.Inf(1)},
    }

    for _, tt := range tests {
        t.Run(tt.name, func(t *testing.T) {
            got := Abs(tt.input)
            if math.IsNaN(tt.want) {
                if !math.IsNaN(got) {
                    t.Errorf("Abs(%v) = %v, want NaN", tt.input, got)
                }
            } else if got != tt.want {
                t.Errorf("Abs(%v) = %v, want %v", tt.input, got, tt.want)
            }
        })
    }
}

func TestMax(t *testing.T) {
    tests := []struct {
        name string
        a, b float64
        want float64
    }{
        {"正数: a > b", 10.0, 5.0, 10.0},
        {"正数: a < b", 3.0, 7.0, 7.0},
        {"正数: a = b", 5.0, 5.0, 5.0},
        {"负数: a > b", -2.0, -5.0, -2.0},
        {"负数: a < b", -8.0, -3.0, -3.0},
        {"一正一负", -5.0, 5.0, 5.0},
        {"零值", 0.0, 0.0, 0.0},
        {"与零比较正", 10.0, 0.0, 10.0},
        {"与零比较负", -5.0, 0.0, 0.0},
        {"小数比较", 2.5, 3.5, 3.5},
        {"特殊值: 无穷大", math.Inf(1), 1000, math.Inf(1)},
        {"特殊值: 负无穷大", math.Inf(-1), -1000, -1000},
    }

    for _, tt := range tests {
        t.Run(tt.name, func(t *testing.T) {
            got := Max(tt.a, tt.b)
            if math.IsNaN(tt.want) {
                if !math.IsNaN(got) {
                    t.Errorf("Max(%v, %v) = %v, want NaN", tt.a, tt.b, got)
                }
            } else if got != tt.want {
                t.Errorf("Max(%v, %v) = %v, want %v", tt.a, tt.b, got, tt.want)
            }
        })
    }
}

// 测试 NaN 情况的辅助函数
func TestAbsWithNaN(t *testing.T) {
    t.Run("NaN input", func(t *testing.T) {
        got := Abs(math.NaN())
        if !math.IsNaN(got) {
            t.Errorf("Abs(NaN) = %v, want NaN", got)
        }
    })
}

func TestMaxWithNaN(t *testing.T) {
    tests := []struct {
        name string
        a, b float64
    }{
        {"第一个参数为NaN", math.NaN(), 5.0},
        {"第二个参数为NaN", 5.0, math.NaN()},
        {"两个参数都为NaN", math.NaN(), math.NaN()},
    }

    for _, tt := range tests {
        t.Run(tt.name, func(t *testing.T) {
            got := Max(tt.a, tt.b)
            if !math.IsNaN(got) {
                t.Errorf("Max(%v, %v) = %v, want NaN", tt.a, tt.b, got)
            }
        })
    }
}

// 边界值测试
func TestAbsBoundaryValues(t *testing.T) {
    tests := []struct {
        name  string
        input float64
        want  float64
    }{
        {"最大浮点数", math.MaxFloat64, math.MaxFloat64},
        {"最小标准浮点数", math.SmallestNonzeroFloat64, math.SmallestNonzeroFloat64},
        {"负的最大浮点数", -math.MaxFloat64, math.MaxFloat64},
    }

    for _, tt := range tests {
        t.Run(tt.name, func(t *testing.T) {
            got := Abs(tt.input)
            if got != tt.want {
                t.Errorf("Abs(%v) = %v, want %v", tt.input, got, tt.want)
            }
        })
    }
}

func TestMaxBoundaryValues(t *testing.T) {
    tests := []struct {
        name string
        a, b float64
        want float64
    }{
        {"最大浮点数比较", math.MaxFloat64, math.MaxFloat64 / 2, math.MaxFloat64},
        {"最小浮点数比较", math.SmallestNonzeroFloat64, math.SmallestNonzeroFloat64 * 2, math.SmallestNonzeroFloat64 * 2},
    }

    for _, tt := range tests {
        t.Run(tt.name, func(t *testing.T) {
            got := Max(tt.a, tt.b)
            if got != tt.want {
                t.Errorf("Max(%v, %v) = %v, want %v", tt.a, tt.b, got, tt.want)
            }
        })
    }
}

func TestMin(t *testing.T) {
	tests := []struct {
		name string
		a, b float64
		want float64
	}{
		{"正数: a > b", 10.0, 5.0, 5.0},
        {"正数: a < b", 3.0, 7.0, 3.0},
        {"正数: a = b", 5.0, 5.0, 5.0},
        {"负数: a > b", -2.0, -5.0, -5.0},
        {"负数: a < b", -8.0, -3.0, -8.0},
        {"一正一负", -5.0, 5.0, -5.0},
        {"零值", 0.0, 0.0, 0.0},
        {"与零比较正", 10.0, 0.0, 0.0},
        {"与零比较负", -5.0, 0.0, -5.0},
        {"小数比较", 2.5, 3.5, 2.5},
        {"特殊值: 无穷大", math.Inf(1), 1000, 1000},
        {"特殊值: 负无穷大", math.Inf(-1), -1000, math.Inf(-1)},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := Min(tt.a, tt.b)
			if got != tt.want {
				t.Errorf("Min(%v, %v) = %v, want %v", tt.a, tt.b, got, tt.want)
			}
		})
	}
}

// 测试 RandInt 基本功能
func TestRandInt_Basic(t *testing.T) {
    tests := []struct {
        name string
        seed int64
    }{
        {"Seed 1", 1},
        {"Seed 42", 42},
        {"Seed 123", 123},
        {"Seed current time", time.Now().UnixNano()},
    }
    
    for _, tt := range tests {
        t.Run(tt.name, func(t *testing.T) {
            rand.Seed(tt.seed)
            result := RandInt()
            
            // 基本验证
            if result < 0 {
                t.Errorf("Expected non-negative number, got: %d", result)
            }
            
            t.Logf("With seed %d, got random number: %d", tt.seed, result)
        })
    }
}

// 测试 RandInt 的随机性（统计测试）
func TestRandInt_Randomness(t *testing.T) {
    rand.Seed(time.Now().UnixNano())
    
    const numSamples = 1000
    values := make([]int, numSamples)
    
    // 收集样本
    for i := 0; i < numSamples; i++ {
        values[i] = RandInt()
    }
    
    // 检查是否都在合理范围内
    for _, val := range values {
        if val < 0 {
            t.Errorf("Found negative random number: %d", val)
        }
    }
    
    // 简单检查多样性（非确定性测试）
    uniqueValues := make(map[int]bool)
    for _, val := range values {
        uniqueValues[val] = true
    }
    
    uniquenessRatio := float64(len(uniqueValues)) / float64(numSamples)
    t.Logf("Uniqueness ratio: %.2f%%", uniquenessRatio*100)
    
    if uniquenessRatio < 0.5 {
        t.Log("Warning: Low uniqueness in random values")
    }
}

// 并发安全测试
func TestRandInt_Concurrent(t *testing.T) {
    rand.Seed(1)
    
    const goroutines = 10
    const iterations = 100
    
    results := make(chan int, goroutines*iterations)
    errors := make(chan error, goroutines)
    
    // 启动多个 goroutine 并发调用
    for i := 0; i < goroutines; i++ {
        go func() {
            for j := 0; j < iterations; j++ {
                result := RandInt()
                if result < 0 {
                    errors <- fmt.Errorf("Negative random number: %d", result)
                    return
                }
                results <- result
            }
            errors <- nil
        }()
    }
    
    // 收集错误
    for i := 0; i < goroutines; i++ {
        if err := <-errors; err != nil {
            t.Errorf("Concurrency test failed: %v", err)
        }
    }
    
    close(results)
    
    // 验证所有结果
    count := 0
    for result := range results {
        if result < 0 {
            t.Errorf("Concurrent test produced negative number: %d", result)
        }
        count++
    }
    
    if count != goroutines*iterations {
        t.Errorf("Expected %d results, got %d", goroutines*iterations, count)
    }
}

func BenchmarkRandInt(b *testing.B) {
    for i := 0; i < b.N; i++ {
        RandInt()
    }
}

func TestPrintHello(t *testing.T) {
	tests := []struct {
		name string
		input string
		output string
	} {
		{"test", "helloworld", "helloworld"},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T){
			if got := PrintHello(); got != "Hello World" {
				t.Errorf("PrintHello() = %v, want %v", got, "Hello World")
			}
			assert.Equal(t,"Hello World", PrintHello())
		})
	}
}