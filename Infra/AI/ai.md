## AI Infra

## 2026-03-02 AutoDL RTX 5090 首次完整成功部署 vLLM

**环境**：
- 镜像：PyTorch 2.8.0 + CUDA 12.8 + Python 3.12
- GPU：RTX 5090 单卡

![env](./images/env_1.png)

**关键环境变量**（~/.bashrc）：
```bash
export HF_ENDPOINT=https://hf-mirror.com
export HF_HUB_ENABLE_HF_TRANSFER=1
```

**安装命令**：
```bash
pip install hf_transfer -U
pip install vllm -U --extra-index-url https://download.pytorch.org/whl/cu128
```

**启动命令**：
```bash
vllm serve Qwen/Qwen2.5-1.5B-Instruct \
  --port 8000 \
  --tensor-parallel-size 1 \
  --gpu-memory-utilization 0.85 \
  --max-model-len 8192 \
  --enforce-eager
```

![vllm](./images/env_2.png)

**验证**：
- curl /v1/models -> 返回模型列表
- curl /v1/chat/completions -> 正常中文回复

![vllm_test](./images/env_3.png)

**安装k3s**

**配置kubectl**

**验证**
