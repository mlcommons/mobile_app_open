import { ReasonPhrases, StatusCodes } from 'http-status-codes';

import * as functions from "firebase-functions";

import * as admin from "firebase-admin";
admin.initializeApp();
const db = admin.firestore();

// exports.writeToFirestore = functions.firestore
//   .document('some/doc')
//   .onWrite((change, context) => {
//     db.doc('some/otherdoc').set({ ... });
//   });

import { Convert, ExtendedResult } from "./json-parser";

////////////////////////////////////////// https://app.quicktype.io/
////////////////////////////////////////// https://app.quicktype.io/
////////////////////////////////////////// https://app.quicktype.io/
////////////////////////////////////////// https://app.quicktype.io/
////////////////////////////////////////// https://app.quicktype.io/

export const upload = functions.https.onRequest(async (request, response) => {
  try {
    const token = request.headers['authorization'];
    console.log(`token ${token}`);
    if (token == undefined) {
      throw 'Authorization missing';
    }
    const userData = await admin.auth().verifyIdToken(token);
    console.log(`user identified: ${userData.uid}`);
  } catch (e) {
    console.log(`auth failed: ${e}`);
    response.status(StatusCodes.UNAUTHORIZED).send(ReasonPhrases.UNAUTHORIZED);
    return;
  }
  let data: ExtendedResult = request.body;
  console.log(data);
  functions.logger.info(data, {structuredData: true});
  try {
    let parsed = Convert.toExtendedResult(request.rawBody.toString());
    let docRef = db.doc(`v0/${ parsed.uuid }`);
    await docRef.set(parsed);
    response.status(StatusCodes.OK).send('success!');
  } catch(e) {
    response.status(StatusCodes.BAD_REQUEST).send('invalid json');
    return;
  }
});
