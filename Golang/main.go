package main

import (
	"fmt"
	"log"
	"net/http"
	"sync"
	"time"

	"github.com/gin-contrib/cors"
	"github.com/gin-contrib/requestid"
	"github.com/gin-gonic/gin"
	"github.com/google/uuid"
	"golang.org/x/sync/errgroup"
)

type Product struct {
	Username    string    `json:"username" binding:"required"`
	Name        string    `json:"name" binding:"required"`
	Category    string    `json:"category" binding:"required"`
	Price       int       `json:"price" binding:"gte=0"`
	Description string    `json:"description"`
	CreatedAt   time.Time `json:"createdAt"`
}

type productHandler struct {
	sync.RWMutex
	products map[string]Product
}

func newProductHandler() *productHandler {
	return &productHandler{
		products: make(map[string]Product),
	}
}

func (u *productHandler) Create(c *gin.Context) {
	u.Lock()
	defer u.Unlock()

	// 1. 参数解析
	var product Product
	if err := c.ShouldBindJSON(&product); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	// 2. 参数校验
	if _, ok := u.products[product.Name]; ok {
		c.JSON(http.StatusBadRequest, gin.H{"error": fmt.Sprintf("product %s already exist", product.Name)})
		return
	}
	product.CreatedAt = time.Now()

	// 3. 逻辑处理
	u.products[product.Name] = product
	log.Printf("Register product %s success", product.Name)

	// 4. 返回结果
	c.JSON(http.StatusOK, product)
}

func (u *productHandler) Get(c *gin.Context) {
	u.Lock()
	defer u.Unlock()

	product, ok := u.products[c.Param("name")]
	if !ok {
		c.JSON(http.StatusNotFound, gin.H{"error": fmt.Errorf("can not found product %s", c.Param("name"))})
		return
	}

	c.JSON(http.StatusOK, product)
}

func Logger() gin.HandlerFunc {
	return func(c *gin.Context) {
		t := time.Now()

		c.Set("requestStartTime", t)

		c.Next()

		latency := time.Since(t)
		log.Printf("[INFO]%s %s %v", c.Request.Method, c.Request.URL.Path, latency)

		status := c.Writer.Status()

		log.Println(status)
	}
}

func router() http.Handler {
	router := gin.Default()

	// gin 中通过 gin.Engine的Use方法来加载中间件, 中间件可以加载到不同的位置上, 而且不同的位置作用范围也不同
	// 中间件作用于所有的HTTP请求
	// gin 框架本身支持的一些中间件
	// gin.Logger() 日志中间件
	// gin.Recovery() 恢复中间件
	// gin.CustomRecovery(handle gin.RecoveryFunc) 恢复时还会调用传入的handle方法进行处理
	// gin.BasicAuth(): HTTP请求基本认证
	router.Use(Logger(), gin.Recovery())

	router.Use(requestid.New(requestid.WithGenerator(func() string {
		return "request-id-" + uuid.New().String()
	})))

	// 跨域中间件
	router.Use(cors.New(cors.Config{
		AllowOrigins:     []string{"https://foo.com"},
		AllowMethods:     []string{"PUT", "PATCH"},
		AllowHeaders:     []string{"Origin"},
		ExposeHeaders:    []string{"Content-Length"},
		AllowCredentials: true,
		AllowOriginFunc: func(origin string) bool {
			return origin == "https://github.com"
		},
		MaxAge: 12 * time.Hour,
	}))

	router.GET("/test", func(c *gin.Context) {
		example := c.MustGet("requestStartTime").(time.Time)
		log.Println(example)
	})

	productHandler := newProductHandler()
	// 路由分组、中间件、认证
	v1 := router.Group("/v1")
	{
		productv1 := v1.Group("/products")
		{
			// 路由匹配
			productv1.POST("", productHandler.Create)
			productv1.GET(":name", productHandler.Get)
		}
	}
	// 这里的认证作用于v2分组下的所有路由
	v2 := router.Group("/v2", gin.BasicAuth(gin.Accounts{"foo": " bar", "colin": "colin404"}))
	{
		productv2 := v2.Group("/products")
		{
			productv2.POST("", productHandler.Create)
			productv2.GET(":name", productHandler.Get)
		}
	}

	return router
}

func main() {
	var eg errgroup.Group

	// 一进程多端口
	insecureServer := &http.Server{
		Addr:         ":8080",
		Handler:      router(),
		ReadTimeout:  5 * time.Second,
		WriteTimeout: 10 * time.Second,
	}

	secureServer := &http.Server{
		Addr:         ":8443",
		Handler:      router(),
		ReadTimeout:  5 * time.Second,
		WriteTimeout: 10 * time.Second,
	}

	eg.Go(func() error {
		err := insecureServer.ListenAndServe()
		if err != nil && err != http.ErrServerClosed {
			log.Fatal(err)
		}
		return err
	})

	eg.Go(func() error {
		err := secureServer.ListenAndServeTLS("server.pem", "server.key")
		if err != nil && err != http.ErrServerClosed {
			log.Fatal(err)
		}
		return err
	})

	if err := eg.Wait(); err != nil {
		log.Fatal(err)
	}
}

