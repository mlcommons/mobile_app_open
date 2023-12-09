import React from "react";
import { ChakraProvider } from "@chakra-ui/react";
import { BrowserRouter } from "react-router-dom";
import "./App.css";
import theme from "./styles/theme";
import Router from "./modules/Router";
import AppQueryClient from "./AppQueryClient";
import { ReactQueryDevtools } from "@tanstack/react-query-devtools";

function App() {
  return (
    <ChakraProvider theme={theme}>
      <AppQueryClient>
        <BrowserRouter>
          <Router />
        </BrowserRouter>
        <ReactQueryDevtools initialIsOpen={false} />
      </AppQueryClient>
    </ChakraProvider>
  );
}

export default App;
