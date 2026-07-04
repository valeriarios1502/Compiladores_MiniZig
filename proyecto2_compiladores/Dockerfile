FROM node:22-bookworm-slim AS builder

WORKDIR /app

RUN apt-get update \
    && apt-get install -y --no-install-recommends g++ \
    && rm -rf /var/lib/apt/lists/*

COPY ast.cpp ast.h environment.h main.cpp parser.cpp parser.h scanner.cpp scanner.h token.cpp token.h Typechecker.cpp Typechecker.h visitor.cpp visitor.h ./

RUN mkdir -p build \
    && g++ -std=c++14 -O2 -o build/Proyecto2 \
      main.cpp scanner.cpp token.cpp parser.cpp ast.cpp visitor.cpp Typechecker.cpp

FROM node:22-bookworm-slim AS runtime

ENV NODE_ENV=production
ENV PORT=3001

WORKDIR /app

COPY package.json server.js ./
COPY backend ./backend
COPY --from=builder /app/build/Proyecto2 ./build/Proyecto2

EXPOSE 3001

CMD ["node", "server.js"]
