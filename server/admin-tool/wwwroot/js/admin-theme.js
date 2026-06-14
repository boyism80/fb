window.fbAdminTheme = {
    storageKey: 'fb-admin-theme',
    _mediaListener: null,

    resolve: function (theme) {
        if (theme === 'system') {
            return window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light';
        }

        return theme === 'light' ? 'light' : 'dark';
    },

    get: function () {
        var theme = localStorage.getItem(this.storageKey);
        if (theme === 'light' || theme === 'dark' || theme === 'system') {
            return theme;
        }

        return 'dark';
    },

    getEffective: function () {
        return this.resolve(this.get());
    },

    apply: function (theme) {
        var preference = theme === 'light' || theme === 'system' ? theme : 'dark';
        var resolved = this.resolve(preference);

        document.documentElement.setAttribute('data-theme', resolved);
        document.documentElement.setAttribute('data-theme-preference', preference);

        if (document.body) {
            document.body.setAttribute('data-bs-theme', resolved === 'light' ? 'light' : 'dark');
        }
    },

    set: function (theme) {
        var preference = theme === 'light' || theme === 'system' ? theme : 'dark';
        localStorage.setItem(this.storageKey, preference);
        this.apply(preference);
    },

    init: function () {
        this.apply(this.get());
        this.bindSystemListener();
    },

    bindSystemListener: function () {
        if (this._mediaListener) {
            return;
        }

        var self = this;
        this._mediaListener = function () {
            if (self.get() === 'system') {
                self.apply('system');
            }
        };

        window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', this._mediaListener);
    }
};

document.addEventListener('DOMContentLoaded', function () {
    window.fbAdminTheme.init();
});
