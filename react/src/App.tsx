import React from "react";
import { ChakraProvider } from "@chakra-ui/react";
import { BrowserRouter } from "react-router-dom";
import "./App.css";
import theme from "./styles/theme";
import Router from "./modules/Router";
import AppQueryClient from "./AppQueryClient";
import store, { persistedStore } from "./store";
import { Provider as ReduxProvider } from "react-redux";
import { PersistGate } from "redux-persist/integration/react";

function App() {
  return (
    <ChakraProvider theme={theme}>
      <ReduxProvider store={store}>
        <AppQueryClient>
          <PersistGate loading={null} persistor={persistedStore}>
            <BrowserRouter>
              <Router />
            </BrowserRouter>
          </PersistGate>
        </AppQueryClient>
      </ReduxProvider>
    </ChakraProvider>
  );
}

export default App;
