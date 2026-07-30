{{- define "apiway.image" -}}
{{- printf "%s/%s:%s" .root.Values.image.registry .name (.root.Values.image.tag) -}}
{{- end -}}

{{- define "apiway.secretName" -}}
{{- if .Values.jwt.existingSecret -}}
{{- .Values.jwt.existingSecret -}}
{{- else -}}
apiway-secrets
{{- end -}}
{{- end -}}
