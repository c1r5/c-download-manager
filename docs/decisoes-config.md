# Decisao: Modulo de Configuracao

## Formato
- Arquivo `.ini` com secoes `[topic]` e pares `CHAVE=VALOR`
- Localizado em `~/.config/cdownload-manager/config.ini`
- Criado automaticamente com valores padrao na primeira execucao

## Exemplo do arquivo gerado
```ini
[download]
max_connections=8
max_downloads=3
max_retries=3
```

## Configuracoes

| Chave | Default | Descricao |
|-------|---------|-----------|
| max_connections | 8 | Maximo de conexoes simultaneas por download |
| max_downloads | 3 | Maximo de downloads ao mesmo tempo |
| max_retries | 3 | Maximo de retentativas |

## Justificativa
- Formato INI e simples, legivel e nao requer dependencias externas
- XDG Base Directory (`~/.config/`) e o padrao Linux para arquivos de configuracao
- `max_connections` substitui a constante hardcoded `MAX_CONNECTIONS` de `constants.h`
- Parser simples que ignora linhas de comentario (#) e secoes ([])
