import { signIn } from "../../../services/firebaseAuthService";
export const loginUser = async (email: string, password: string) => {
  return signIn(email, password);
};
