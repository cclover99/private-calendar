import React from "react";
import { renderToReadableStream } from "react-dom/server";
import { App } from "./src/App";

const server = Bun.serve({
  routes: {
    "/api/hello": {
      GET: () => Response.json({ message: "Hello, world!", method: "GET" }),
      PUT: () => Response.json({ message: "Hello, world!", method: "PUT" }),
    },
    "/api/hello/:name": req =>
      Response.json({ message: `Hello, ${req.params.name}!` }),
    "/api/*": () => Response.json({ error: "Not found" }, { status: 404 }),
  },

  async fetch(req) {
    const { pathname } = new URL(req.url);

    if (pathname !== "/") {
      for (const dir of ["dist", "public"]) {
        const file = Bun.file(`${import.meta.dir}/${dir}${pathname}`);
        if (await file.exists()) return new Response(file);
      }
    }

    const stream = await renderToReadableStream(React.createElement(App), {
      bootstrapModules: ["/frontend.js"],
      onError: console.error,
    });

    return new Response(stream, {
      headers: { "Content-Type": "text/html; charset=utf-8" },
    });
  },
});

console.log(`🚀 Server running at ${server.url}`);