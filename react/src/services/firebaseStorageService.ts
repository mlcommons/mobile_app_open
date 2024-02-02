import { ref, listAll, getDownloadURL, getStorage } from "firebase/storage";
import { app } from "../firebase-config";

const storage = getStorage(app);

const listItems = async (path: string) => {
  const listRef = ref(storage, path);
  return listAll(listRef)
    .then((res) => {
      return res.items.map((item) => {
        return {
          name: item.name,
          path: item.fullPath,
        };
      });
    })
    .catch((error) => {
      return [];
    });
};

const getDownloadUrl = async (itemPath: string) => {
  return getDownloadURL(ref(storage, itemPath));
};

export { listItems, getDownloadUrl };
