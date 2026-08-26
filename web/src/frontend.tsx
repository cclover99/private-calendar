import { StrictMode } from "react";
import { hydrateRoot } from "react-dom/client";
import { App } from "./App";

const app = (
  <StrictMode>
    <App />
  </StrictMode>
);

hydrateRoot(document, app);