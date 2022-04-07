import { Request, Response, NextFunction } from 'express';
import * as functions from 'firebase-functions';
import * as admin from 'firebase-admin';
import { ReasonPhrases, StatusCodes } from 'http-status-codes';

export function auth(
  firebaseAuth: admin.auth.Auth,
): (request: Request, response: Response, next: NextFunction) => Promise<void> {
  return async (request: Request, response: Response, next: NextFunction) => {
    try {
      const token = request.headers['authorization'];
      if (token == undefined) {
        throw Error('Authorization missing');
      }
      await firebaseAuth.verifyIdToken(token);
      next();
    } catch (e) {
      functions.logger.warn(`auth failed: ${e}`);
      response
        .status(StatusCodes.UNAUTHORIZED)
        .send(ReasonPhrases.UNAUTHORIZED);
      return;
    }
  };
}
