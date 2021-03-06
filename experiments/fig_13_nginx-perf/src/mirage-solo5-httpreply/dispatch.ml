open Lwt.Infix

(** Common signature for http and https. *)
module type HTTP = Cohttp_lwt.S.Server

(* Logging *)
let https_src = Logs.Src.create "https" ~doc:"HTTPS server"
module Https_log = (val Logs.src_log https_src : Logs.LOG)

let http_src = Logs.Src.create "http" ~doc:"HTTP server"
module Http_log = (val Logs.src_log http_src : Logs.LOG)

module Dispatch (FS: Mirage_kv.RO) (S: HTTP) = struct

  let failf fmt = Fmt.kstrf Lwt.fail_with fmt

  (* given a URI, find the appropriate file,
   * and construct a response with its contents. *)
  let rec dispatcher fs uri =
    match Uri.path uri with
    | "" | "/" -> dispatcher fs (Uri.with_path uri "index.html")
    | path ->
      let header =
        Cohttp.Header.init_with "Strict-Transport-Security" "max-age=31536000"
      in
      let mimetype = Magic_mime.lookup path in
      let headers = Cohttp.Header.add header "content-type" mimetype in
      Lwt.catch
        (fun () ->
           FS.get fs (Mirage_kv.Key.v path) >>= function
           | Error e -> failf "get: %a" FS.pp_error e
           | Ok body ->
             S.respond_string ~status:`OK ~body ~headers ())
        (fun _exn ->
           S.respond_not_found ())

  let serve dispatch =
    let callback (_, cid) request _body =
      let uri = Cohttp.Request.uri request in
      dispatch uri
    in
    S.make ~callback ()

end

module HTTPS
    (Pclock: Mirage_clock.PCLOCK) (DATA: Mirage_kv.RO) (KEYS: Mirage_kv.RO) (Http: HTTP) =
struct

  module X509 = Tls_mirage.X509(KEYS)(Pclock)
  module D = Dispatch(DATA)(Http)

  let tls_init kv =
    X509.certificate kv `Default >>= fun cert ->
    let conf = Tls.Config.server ~certificates:(`Single cert) () in
    Lwt.return conf

  let start _clock data keys http =
    tls_init keys >>= fun _ ->
    let http_port = Key_gen.http_port () in
    let tcp = `TCP http_port in
    let http =
      Http_log.info (fun f -> f "listening on %d/TCP" http_port);
      http tcp @@ D.serve (D.dispatcher data)
    in
    Lwt.join [ http ]

end
