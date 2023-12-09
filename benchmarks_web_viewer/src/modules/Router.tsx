import * as React from "react";
import { Navigate, Route, Routes } from "react-router-dom";
import { PropsWithChildren, Suspense } from "react";
import CenterSpinner from "../sharedComponents/CenterSpinner";
import { useUser } from "./auth/hooks/useUser";
import { useIsRestoring } from "@tanstack/react-query";

// import useUser from "./auth/store/user.state";

const Login = React.lazy(() => import("./auth/components/Login.page"));

const BenchmarkList = React.lazy(
  () => import("./benchmarks/components/BenchmarkList.page"),
);
const BenchmarkDetailsPage = React.lazy(
  () => import("./benchmarks/components/BenchmarkDetails.page"),
);

const GuardedRoute = ({ children }: PropsWithChildren) => {
  const user = useUser();
  const isRestoring = useIsRestoring();
  if (isRestoring) return <CenterSpinner />;
  if (!user) return <Navigate to="/auth/sign-in" replace />;
  return <>{children}</>;
};

const Router = () => {
  return (
    <Suspense fallback={<CenterSpinner />}>
      <Routes>
        <Route path="/auth/sign-in" element={<Login />} />
        <Route
          path="/"
          element={
            <GuardedRoute>
              <BenchmarkList />
            </GuardedRoute>
          }
        />
        <Route
          path="/benchmarks"
          element={
            <GuardedRoute>
              <BenchmarkList />
            </GuardedRoute>
          }
        />
        <Route
          path="/benchmarks/:id"
          element={
            <GuardedRoute>
              <BenchmarkDetailsPage />
            </GuardedRoute>
          }
        />
      </Routes>
    </Suspense>
  );
};

export default Router;
