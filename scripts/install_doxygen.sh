#!/bin/bash
set -e

sudo apt update

sudo apt install -y doxygen graphviz

echo "✅ Instalação concluída"
echo "Doxygen version:"
doxygen --version
echo "Graphviz version:"
dot -V
