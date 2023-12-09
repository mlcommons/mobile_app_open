import { initializeApp } from "firebase/app";

const firebaseConfig = {
  apiKey: "AIzaSyBMq6_1X-G9nzTaozrFaA1MtnPjaXwzmxQ",
  authDomain: "mlcommons-8be22.firebaseapp.com",
  databaseURL: "https://mlcommons-8be22-default-rtdb.firebaseio.com",
  projectId: "mlcommons-8be22",
  storageBucket: "mlcommons-8be22.appspot.com",
  messagingSenderId: "410781328311",
  appId: "1:410781328311:web:d653957baba0bb4607fe32",
  measurementId: "G-KJXBXVE7WV",
};

const app = initializeApp(firebaseConfig);

export { app };
