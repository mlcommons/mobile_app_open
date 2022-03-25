import { Request, Response, NextFunction } from 'express';
import * as functions from 'firebase-functions';
import { ReasonPhrases, StatusCodes } from 'http-status-codes';
import { Convert } from './extended-result.gen';

export function upload(db: FirebaseFirestore.Firestore): (request: Request, response: Response, next: NextFunction) => Promise<void> {
  return async (request: Request, response: Response, next: NextFunction) => {
    try {
      let parsed = Convert.toExtendedResult(request.body);
      if (parsed.uploadDate != '') {
        throw 'uploadDate must be empty';
      }
      parsed.uploadDate = new Date().toISOString();
      let docRef = db.doc(`v0/${ parsed.uuid }`);
      const alreadyExists = (await docRef.get()).exists;
      if (alreadyExists) {
        response.status(StatusCodes.CONFLICT).send('result already exists in database');
        return;
      }
      await docRef.set(parsed);
      response.status(StatusCodes.CREATED).send(ReasonPhrases.OK);
    } catch(e) {
      functions.logger.info(e, {structuredData: true});
      response.status(StatusCodes.BAD_REQUEST).send('invalid json: ' + e);
      return;
    }
  }
}
