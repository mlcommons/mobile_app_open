const convertBlobToJson = (blob: Blob) => {
  return new Promise((resolve, reject) => {
    const reader = new FileReader();
    reader.onload = () => {
      if (reader.result) {
        try {
          const result = JSON.parse(reader.result as string);
          resolve(result);
        } catch (error) {
          console.error("Error parsing JSON:", error);
          reject(error);
        }
      }
      return null;
    };

    reader.onerror = (error) => {
      reject(error);
    };

    reader.readAsText(blob);
  });
};

export { convertBlobToJson };
