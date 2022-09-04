import { Request, Response, NextFunction } from 'express';
import * as functions from 'firebase-functions';
import { StatusCodes } from 'http-status-codes';
import { osListReverse } from './indices';
import { currentPrefix } from './prefix.gen';
import { IncomingHttpHeaders } from 'http';

function _makeBatchQuery(
  db: FirebaseFirestore.Firestore,
  pageSize: number,
): FirebaseFirestore.Query<FirebaseFirestore.DocumentData> {
  return db
    .collection(`/${currentPrefix}/`)
    .orderBy('data.meta.upload_date', 'desc')
    .limit(pageSize);
}

function _getPageSize(headers: IncomingHttpHeaders): number {
  const pageSizeString = headers['page-size'];
  if (typeof pageSizeString != 'string') {
    throw Error('page-size header is missing');
  }
  const maxAllowedPageSize = 50;
  const result = parseInt(pageSizeString);
  if (result > maxAllowedPageSize) {
    throw Error('page size is too big');
  }
  return result;
}

export function fetchNext(
  db: FirebaseFirestore.Firestore,
): (request: Request, response: Response, next: NextFunction) => Promise<void> {
  return async (request: Request, response: Response, _: NextFunction) => {
    try {
      const pageSize = _getPageSize(request.headers);

      let query = _makeBatchQuery(db, pageSize);

      const cursor = request.headers['uuid-cursor'];
      if (typeof cursor == 'string' && cursor != '') {
        const cursorRef = db.doc(`v0/${cursor}`);
        const cursorSnapshot = await cursorRef.get();
        query = query.startAfter(cursorSnapshot);
      }

      const excludedOs = request.headers['where-os-is-not'];
      if (typeof excludedOs == 'string' && excludedOs != '') {
        const excludedOsArray = excludedOs.split(',');
        for (const index in excludedOsArray) {
          const os = excludedOsArray[index];
          const osIndex = osListReverse[os];
          if (osIndex == undefined) {
            throw Error(`unknown os: ${os}`);
          }
          query = query.where(`indices.osIs.${osIndex}`, '==', false);
        }
      }

      const snapshot = await query.get();
      const result = [];
      for (const document of snapshot.docs) {
        result.push(document.data().data);
      }
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
      const pageSize = _getPageSize(request.headers);

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
