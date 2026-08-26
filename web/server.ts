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

        if (pathname === "/index" || pathname === "/index.html") {
            return Response.redirect("/", 308);
        }

        if (pathname !== "/") {
            const ASSET_DIRS = ["dist", "public"];
            
            for (const dir of ASSET_DIRS) {
                const file = Bun.file(`${import.meta.dir}/${dir}${pathname}`);
                if (await file.exists()) return new Response(file);
            }
            return new Response("Not found", { status: 404 });
        
        }

        const stream = await renderToReadableStream(React.createElement(App), {
            bootstrapModules: ["/frontend.js"],
            onError: console.error,
        });

        return new Response(stream, {
            headers: { "Content-Type": "text/html; charset=utf-8" },
        });
    }
});

console.log(`🚀 Server running at ${server.url}`);