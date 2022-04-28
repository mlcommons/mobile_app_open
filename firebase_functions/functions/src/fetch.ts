import { Request, Response, NextFunction } from 'express';
import * as functions from 'firebase-functions';
import { StatusCodes } from 'http-status-codes';
import { currentPrefix } from './prefix.gen';

function _makeBatchQuery(
  db: FirebaseFirestore.Firestore,
  pageSize: number,
): FirebaseFirestore.Query<FirebaseFirestore.DocumentData> {
  return db
    .collection(`/${currentPrefix}/`)
    .orderBy('meta.upload_date', 'desc')
    .limit(pageSize);
}

export function fetchFirst(
  db: FirebaseFirestore.Firestore,
): (request: Request, response: Response, next: NextFunction) => Promise<void> {
  return async (request: Request, response: Response, _: NextFunction) => {
    try {
      const pageSizeString = request.headers['page-size'];
      if (typeof pageSizeString != 'string') {
        throw Error('page-size header is missing');
      }
      const pageSize = parseInt(pageSizeString);
      console.log(`pageSize is ${pageSize}`);

      const snapshot = await _makeBatchQuery(db, pageSize).get();
      const result = [];
      for (const document of snapshot.docs) {
        result.push(document.data());
      }
      console.log(result);
      response.status(StatusCodes.OK).send(JSON.stringify(result));
    } catch (e) {
      functions.logger.info(e, { structuredData: true });
      response.status(StatusCodes.BAD_REQUEST).send('invalid request: ' + e);
      return;
    }
  };
}

export function fetchNext(
  db: FirebaseFirestore.Firestore,
): (request: Request, response: Response, next: NextFunction) => Promise<void> {
  return async (request: Request, response: Response, _: NextFunction) => {
    try {
      const pageSizeString = request.headers['page-size'];
      if (typeof pageSizeString != 'string') {
        throw Error('page-size header is missing');
      }
      const pageSize = parseInt(pageSizeString);
      console.log(`pageSize is ${pageSize}`);

      const uuid = request.headers['uuid-cursor'];
      if (typeof uuid != 'string') {
        throw Error('uuid-cursor header is missing');
      }

      const cursorRef = db.doc(`v0/${uuid}`);
      const cursorSnapshot = await cursorRef.get();

      const snapshot = await _makeBatchQuery(db, pageSize)
        .startAfter(cursorSnapshot)
        .get();
      const result = [];
      for (const document of snapshot.docs) {
        result.push(document.data());
      }
      console.log(result);
      response.status(StatusCodes.OK).send(JSON.stringify(result));
    } catch (e) {
      functions.logger.info(e, { structuredData: true });
      response.status(StatusCodes.BAD_REQUEST).send('invalid request: ' + e);
      return;
    }
  };
}

export function fetchPrev(
  db: FirebaseFirestore.Firestore,
): (request: Request, response: Response, next: NextFunction) => Promise<void> {
  return async (request: Request, response: Response, _: NextFunction) => {
    try {
      const pageSizeString = request.headers['page-size'];
      if (typeof pageSizeString != 'string') {
        throw Error('page-size header is missing');
      }
      const pageSize = parseInt(pageSizeString);
      console.log(`pageSize is ${pageSize}`);

      const uuid = request.headers['uuid-cursor'];
      if (typeof uuid != 'string') {
        throw Error('uuid-cursor header is missing');
      }

      const cursorRef = db.doc(`v0/${uuid}`);
      const cursorSnapshot = await cursorRef.get();

      const snapshot = await _makeBatchQuery(db, pageSize)
        .startAfter(cursorSnapshot)
        .get();
      const result = [];
      for (const document of snapshot.docs) {
        result.push(document.data());
      }
      console.log(result);
      response.status(StatusCodes.OK).send(JSON.stringify(result));
    } catch (e) {
      functions.logger.info(e, { structuredData: true });
      response.status(StatusCodes.BAD_REQUEST).send('invalid request: ' + e);
      return;
    }
  };
}

export function fetchId(
  db: FirebaseFirestore.Firestore,
): (request: Request, response: Response, next: NextFunction) => Promise<void> {
  return async (request: Request, response: Response, _: NextFunction) => {
    try {
      const uuid = request.headers['uuid'];
      if (typeof uuid != 'string') {
        throw 'uuid header is missing';
      }

      const docRef = db.doc(`v0/${uuid}`);
      const snapshot = await docRef.get();

      response.status(StatusCodes.OK).send(JSON.stringify(snapshot.data()));
    } catch (e) {
      functions.logger.info(e, { structuredData: true });
      response.status(StatusCodes.BAD_REQUEST).send('invalid request: ' + e);
      return;
    }
  };
}
