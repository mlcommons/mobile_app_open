import { extendTheme } from "@chakra-ui/react";

const theme = extendTheme({
  fonts: {
    heading: "Montserrat, sans-serif",
    body: "Open Sans, sans-serif",
  },
  colors: {
    brand: {
      panelBg: "#E4ECFC",
      purpleGradient: "linear(136.13deg, #4B21D6 -93.1%, #9536A6 125.01%)",
      yellowGradient:
        "linear-gradient(109.5deg, #EFA537 -138.01%, #F8DB68 109.9%)",
      goldGradient:
        "linear-gradient(99.36deg, #EFA537 -39.65%, #F8DB68 71.68%)",
    },
  },
});

export default theme;
