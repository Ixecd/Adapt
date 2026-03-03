/*
Copyright 2026.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

package v1alpha1

import (
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	corev1 "k8s.io/api/core/v1"
)

// EDIT THIS FILE!  THIS IS SCAFFOLDING FOR YOU TO OWN!
// NOTE: json tags are required.  Any new fields you add must have json tags for the fields to be serialized.

// VLLMServiceSpec 定义了 vLLM 服务的期望状态
type VLLMServiceSpec struct {
	// Model 指定要使用的 HuggingFace 模型 ID (例如: "facebook/opt-125m")
	// +kubebuilder:validation:Required
	Model string `json:"model"`

	// Image 指定 vLLM 镜像版本
	// +optional
	// +kubebuilder:default:="vllm/vllm-openai:latest"
	Image string `json:"image,omitempty"`

	// Replicas 指定推理服务的副本数
	// +optional
	// +kubebuilder:default:=1
	Replicas *int32 `json:"replicas,omitempty"`

	// Resources 定义资源配额（重点是 GPU nvidia.com/gpu）
	// +optional
	Resources corev1.ResourceRequirements `json:"resources,omitempty"`

	// MaxModelLen 指定最大序列长度，对应 vLLM 的 --max-model-len
	// +optional
	MaxModelLen *int32 `json:"maxModelLen,omitempty"`

	// GPUOption 包含 GPU 的特殊配置，如 Tensor Parallel (TP)
	// +optional
	// +kubebuilder:default:=1
	TensorParallelSize *int32 `json:"tensorParallelSize,omitempty"`

	// Env 允许用户传递环境变量，比如 HF_TOKEN 或加速相关的设置
	// +optional
	Env []corev1.EnvVar `json:"env,omitempty"`
}

// VLLMServiceStatus 定义了 vLLM 服务的观测状态
type VLLMServiceStatus struct {
	// ReadyReplicas 当前可用的副本数
	// +optional
	ReadyReplicas int32 `json:"readyReplicas,omitempty"`

	// Endpoint 服务可访问的内部地址 (例如: vllm-service-example.default.svc:8000)
	// +optional
	Endpoint string `json:"endpoint,omitempty"`

	// Phase 当前服务的阶段 (Pending, Running, Failed)
	// +optional
	Phase string `json:"phase,omitempty"`

	// +listType=map
	// +listMapKey=type
	// +optional
	Conditions []metav1.Condition `json:"conditions,omitempty"`
}

// +kubebuilder:object:root=true
// +kubebuilder:subresource:status
// +kubebuilder:printcolumn:name="Model",type="string",JSONPath=".spec.model"
// +kubebuilder:printcolumn:name="Ready",type="integer",JSONPath=".status.readyReplicas"
// +kubebuilder:printcolumn:name="Phase",type="string",JSONPath=".status.phase"

// VLLMService 是 vLLM 操作器的 Schema
type VLLMService struct {
	metav1.TypeMeta   `json:",inline"`
	metav1.ObjectMeta `json:"metadata,omitempty"`

	Spec   VLLMServiceSpec   `json:"spec"`
	Status VLLMServiceStatus `json:"status,omitempty"`
}

// +kubebuilder:object:root=true

type VLLMServiceList struct {
	metav1.TypeMeta `json:",inline"`
	metav1.ListMeta `json:"metadata,omitempty"`
	Items           []VLLMService `json:"items"`
}

func init() {
	SchemeBuilder.Register(&VLLMService{}, &VLLMServiceList{})
}