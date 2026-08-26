import { APITester } from "./APITester";

export function App() {
  return (
    <html lang="en">
      <head>
        <meta charSet="UTF-8" />
        <meta name="viewport" content="width=device-width, initial-scale=1.0" />
        <link rel="icon" type="image/svg+xml" href="/logo.svg" />
        <link rel="stylesheet" href="/index.css"/>
        <title>Calendar</title>
      </head>

      <body>
        <div className="app">
          <div className="logo-container">
            <img src='/logo.svg' alt="Bun Logo" className="logo bun-logo" />
            <img src='/react.svg' alt="React Logo" className="logo react-logo" />
          </div>

          <h1>Bun + React</h1>

          <p>
            Edit <code>src/App.tsx</code> and save to test HMR
          </p>

          <APITester />
        </div>
      </body>
    </html>
  );
}

export default App;