package main

import (
    "text/template"
    "os"
)

const src = `{{- define "test"-}}
模版内容
{{- end -}}

{{ include "test" }}`

func main() {
    tmpl := template.Must(template.New("x").Parse(src))
    tmpl.Execute(os.Stdout, nil)
}