BEGIN   { printf "char license[]=\"\\\n" }
{ gsub(/\"/,"\\\"") ; printf $0"\\n\\\n" }
END     { printf "\";\n" }