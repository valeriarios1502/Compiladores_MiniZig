const { PORT, server } = require("./backend/server")

server.listen(PORT, () => {
  console.log(`Backend MiniZig escuchando en http://localhost:${PORT}`)
})
