import { getAuth, signInWithEmailAndPassword, signOut } from "firebase/auth";
import { app } from "../firebase-config";

const auth = getAuth(app);

const signIn = async (email: string, password: string) => {
  return signInWithEmailAndPassword(auth, email, password)
    .then((userCredential) => {
      // Signed in
      const userObj = userCredential.user;
      return { email: userObj.email || "", uid: userObj.uid };
    })
    .catch((error) => {
      console.log("error", error);
      return null;
    });
};

const logOut = async () => {
  return signOut(auth);
};

export { signIn, logOut };
